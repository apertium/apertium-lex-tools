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

  /*wstring in = L"^patró<n><m><pl>$";
  wstring trad = fstp.biltrans(in);
  wcout << input << L" --> " << trad << endl;*/

  skipUntil(input, output, L'^');
  int val = 0, i = 0;
  bool seenFirst = false;
  wstring sl = L"";
  wstring tl = L"";
  set<wstring> tllu;
  while((val = readGeneration(input, output)) != 0x7fffffff)
  {
    switch(val) 
    { 
      case L'^':
        outOfWord = false;
        fputwc_unlocked(L'i', output);
        break;
      case L'/':
        if(!seenFirst) 
        { 
          seenFirst = true;
          fputwc_unlocked(L'f', output);
        }
        i++;
        wcout << i;
        fputwc_unlocked(L':', output); 
        fputws_unlocked(tl.c_str(), output);
        tl = L"";
        break;
      case L'$':
        outOfWord = true;
        seenFirst = false;
        fputws_unlocked(L"i: ", output);
        fputws_unlocked(sl.c_str(), output);
        sl = L""; tl = L"";       
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


