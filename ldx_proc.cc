/*
 * Copyright (C) 2011 Universitat d'Alacant 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <set>
#include <libgen.h>

#include <lttoolbox/exception.h>
#include <lttoolbox/fst_processor.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>

#define PACKAGE_VERSION "0.1.0"

using namespace std;


int readGeneration(FILE *input, FILE *output);
void skipUntil(FILE *input, FILE *output, wint_t const character);
wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);
wchar_t readEscaped(FILE *input);
void streamError();


FSTProcessor fstp;
bool outOfWord = true;
set<wchar_t> escaped_chars;


void
streamError()
{
  throw Exception("Error: Malformed input stream.");
}

wchar_t
readEscaped(FILE *input)
{
  if(feof(input))
  {
    streamError();
  }

  wchar_t val = static_cast<wchar_t>(fgetwc_unlocked(input));

  if(feof(input) || escaped_chars.find(val) == escaped_chars.end())
  {
    streamError();
  }

  return val;
}


wstring
readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc_unlocked(input));
    result += c;
    if(c != L'\\')
    {
      continue;
    }
    else
    {
      result += static_cast<wchar_t>(readEscaped(input));
    }
  }

  if(c != delim2)
  {
    streamError();
  }

  return result;
}


void
skipUntil(FILE *input, FILE *output, wint_t const character)
{
  while(true)
  {
    wint_t val = fgetwc_unlocked(input);
    if(feof(input))
    {
      return;
    }

    switch(val)
    {
      case L'\\':
        val = fgetwc_unlocked(input);
        if(feof(input))
        {
          return;
        }
        fputwc_unlocked(L'\\', output);
        fputwc_unlocked(val, output);
        break;

      case L'\0':
        fputwc_unlocked(val, output);
        break;

      default:
        if(val == character)
        {
          return;
        }
        else
        {
          fputwc_unlocked(val, output);
        }
        break;
    }
  }
}


int
readGeneration(FILE *input, FILE *output)
{
  wint_t val = fgetwc_unlocked(input);

  if(feof(input))
  {
    return 0x7fffffff;
  }

  if(outOfWord)
  {
    if(val == L'^')
    {
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    else if(val == L'\\')
    {
      fputwc_unlocked(val, output);
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
      fputwc_unlocked(val,output);
      skipUntil(input, output, L'^');
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    else
    {
      fputwc_unlocked(val, output);
      skipUntil(input, output, L'^');
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    outOfWord = false;
  }

  if(val == L'\\')
  {
    val = fgetwc_unlocked(input);
    return static_cast<int>(val);
  }
  else if(val == L'$')
  {
    outOfWord = true;
    return static_cast<int>(L'$');
  }
  else if(val == L'[')
  {
    fputws_unlocked(readFullBlock(input, L'[', L']').c_str(), output);
    return readGeneration(input, output);
  }
  else
  {
    return static_cast<int>(val);
  }

  return 0x7fffffff;
}


int main(int argc, char **argv)
{
  FILE *input = stdin, *output = stdout;

  LtLocale::tryToSetLocale();

  if(argc < 2) 
  { 
    cout << basename(argv[0]) << " v" << PACKAGE_VERSION <<": assign default lexical selection values" << endl;
    cout << "USAGE: " << basename(argv[0]) << " lex_file " << endl;
    exit(-1);
  }

  escaped_chars.insert(L'[');
  escaped_chars.insert(L']');
  escaped_chars.insert(L'{');
  escaped_chars.insert(L'}');
  escaped_chars.insert(L'^');
  escaped_chars.insert(L'$');
  escaped_chars.insert(L'/');
  escaped_chars.insert(L'\\');
  escaped_chars.insert(L'@');
  escaped_chars.insert(L'<');
  escaped_chars.insert(L'>');


  FILE *t_rl = fopen(argv[1], "r");

  fstp.load(t_rl);
  fclose(t_rl);
  fstp.initBiltrans();

  // Input stream is output of lt-proc -b
  // ^patró<n><m><sg>/patron<n><sg>/owner<n><sg>/master<n><sg>/head<n><sg>/pattern<n><sg>/employer<n><sg>$ 
  // Output is disambiguated:
  // ^patró<n><m><sg>/pattern<n><sg>$ 
  // Algorithm:
  // read until '/', then read each from '/' adding to a map, then look up first in transducer, and if the result
  // is found in the map, then output it, otherwise error.

  int val = 0, i = 0;
  bool seenFirst = false;
  wstring sl = L"";
  wstring tl = L"";
  set<wstring> tllu;
  set<wstring> tllu_defaults;

  skipUntil(input, output, L'^');
  outOfWord = false;

  while((val = readGeneration(input, output)) != 0x7fffffff)
  {
    switch(val) 
    { 
      case L'^':
        outOfWord = false;
	val = readGeneration(input, output);
        break;
      case L'/':
        if(!seenFirst) 
        { 
          seenFirst = true;
        } 
        else 
        {
          tllu.insert(tl);
        }
        i++;
        tl = L"";
	val = readGeneration(input, output);
        if(val != L'$')  
        {
          break;
        } 
      case L'$':
        outOfWord = true;
        if(!seenFirst) 
        { 
          seenFirst = true;
        } 
        else 
        {
          tllu.insert(tl);
        }

        seenFirst = false;
        fputws_unlocked(L"^", output);
        fputws_unlocked(sl.c_str(), output);
        if(tllu.size() > 1) 
        {
          tl = L"";
          wstring in = L"^" + sl + L"$";
          wstring trad = fstp.biltrans(in);
          int j = 0;
          bool tlout = false;
          for(set<wstring>::const_iterator it = tllu.begin(), j = tllu.end(); it != j; it++)
          {
            wstring t = L"^" + *it + L"$";
            if(t == trad)
            {
              fputws_unlocked(L"/", output);
              wstring to = t.substr(1, wcslen(t.c_str())-2);
              fputws_unlocked(to.c_str(), output);
              tlout = true;
              break;
            }
          }
        
          j = 0;
          if(!tlout)  // if we haven't found a default translation, then output all
          {
            for(set<wstring>::const_iterator it = tllu.begin(), j = tllu.end(); it != j; it++)
            {
              if(it != tllu.end())
              {
                fputws_unlocked(L"/", output);
              }
              wstring t = *it;
              fputws_unlocked(t.c_str(), output);
            }
          }

        }
        else 
        {
          fputws_unlocked(L"/", output);
          fputws_unlocked(tl.c_str(), output);
        }
        fputws_unlocked(L"$", output);

        sl = L""; tl = L"";       
        tllu.clear();
        i = 0;
        break;
    }
    if(!seenFirst && !outOfWord) 
    {
      sl.append(1, static_cast<wchar_t>(val));
    }
    else if(!outOfWord)
    { 
      tl.append(1, static_cast<wchar_t>(val));
    }
  }   

  return 0;
}


