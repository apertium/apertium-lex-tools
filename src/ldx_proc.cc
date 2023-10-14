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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <set>
#include <libgen.h>

#include <lttoolbox/exception.h>
#include <lttoolbox/fst_processor.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/input_file.h>
#include <unicode/uchar.h>
#include <unicode/ustdio.h>
#include <lttoolbox/i18n.h>

using namespace std;


int32_t readGeneration(InputFile& input, UFILE *output);
void skipUntil(InputFile& input, UFILE *output, UChar32 const character);


FSTProcessor fstp;
bool outOfWord = true;
set<int32_t> escaped_chars;


void
skipUntil(InputFile& input, UFILE* output, UChar32 const character)
{
  while(true)
  {
    UChar32 val = input.get();
    if (input.eof()) {
      return;
    }

    switch(val)
    {
      case '\\':
        val = input.get();
        if (input.eof()) {
          return;
        }
        u_fputc('\\', ouput);
        u_fputc(val, output);
        break;

      case '\0':
        u_fputc(val, output);
        break;

      default:
        if (val == character) {
          return;
        } else {
          u_fputc(val, output);
        }
        break;
    }
  }
}


int32_t
readGeneration(InputFile& input, UFILE* output)
{
  UChar32 val = input.get();

  if (input.eof()) {
    return 0x7fffffff;
  }

  if(outOfWord)
  {
    if(val == '^')
    {
      val = input.get();
      if(input.eof())
      {
        return 0x7fffffff;
      }
    }
    else if(val == '\\')
    {
      u_fputc(val, ouput);
      val = input.get();
      if(input.eof())
      {
        return 0x7fffffff;
      }
      u_fputc(val,output);
      skipUntil(input, output, '^');
      val = input.get();
      if(input.eof())
      {
        return 0x7fffffff;
      }
    }
    else
    {
      u_fputc(val, output);
      skipUntil(input, output, '^');
      val = input.get();
      if(input.eof())
      {
        return 0x7fffffff;
      }
    }
    outOfWord = false;
  }

  if(val == '\\')
  {
    val = input.get();
    return static_cast<int32_t>(val);
  }
  else if(val == '$')
  {
    outOfWord = true;
    return static_cast<int32_t>('$');
  }
  else if(val == '[')
  {
    write(input.readBlock('[', ']'), output);
    return readGeneration(input, output);
  }
  else
  {
    return static_cast<int32_t>(val);
  }

  return 0x7fffffff;
}


int main(int argc, char **argv)
{
  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);

  LtLocale::tryToSetLocale();

  if(argc < 2)
  {
    cout << I18n(ALX_I18N_DATA, "alx").format("ldx_proc_desc", {"program", "version"},
                {basename(argv[0]), PACKAGE_VERSION})
         << endl;
    exit(-1);
  }

  escaped_chars.insert('[');
  escaped_chars.insert(']');
  escaped_chars.insert('{');
  escaped_chars.insert('}');
  escaped_chars.insert('^');
  escaped_chars.insert('$');
  escaped_chars.insert('/');
  escaped_chars.insert('\\');
  escaped_chars.insert('@');
  escaped_chars.insert('<');
  escaped_chars.insert('>');


  FILE *t_rl = fopen(argv[1], "rb");

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

  int32_t val = 0, i = 0;
  bool seenFirst = false;
  UString sl;
  UString tl;
  set<UString> tllu;
  set<UString> tllu_defaults;

  skipUntil(input, output, '^');
  outOfWord = false;

  while((val = readGeneration(input, output)) != 0x7fffffff)
  {
    switch(val)
    {
      case '^':
        outOfWord = false;
        val = readGeneration(input, output);
        break;
      case '/':
        if(!seenFirst)
        {
          seenFirst = true;
        }
        else
        {
          tllu.insert(tl);
        }
        i++;
        tl.clear();
        val = readGeneration(input, output);
        if(val != '$')
        {
          break;
        }
      case '$':
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
        u_fputc('^', output);
        write(sl, output);
        if(tllu.size() > 1)
        {
          tl.clear();
          UString in;
          in += '^';
          in.append(sl);
          in += '$';
          UString trad = fstp.biltrans(in);
          int j = 0;
          bool tlout = false;
          for(auto& it : tllu)
          {
            UString t;
            t += '^';
            t.append(it);
            t += '$';
            if(t == trad)
            {
              u_fputc('/', output);
              write(it, output);
              tlout = true;
              break;
            }
          }

          j = 0;
          if(!tlout)  // if we haven't found a default translation, then output all
          {
            for(auto it = tllu.begin(); it != tllu.end(); ++it)
            {
              if(it != tllu.end())
              {
                u_fputc('/', output);
              }
              write(*it, output);
            }
          }

        }
        else
        {
          u_fputc('/', output);
          write(tl, output);
        }
        u_fputc('$', output);

        sl.clear();
        tl.clear();
        tllu.clear();
        i = 0;
        break;
    }
    if(!seenFirst && !outOfWord)
    {
      sl += static_cast<UChar32>(val);
    }
    else if(!outOfWord)
    {
      tl += static_cast<UChar32>(val);
    }
  }

  return 0;
}
