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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#include <lrx_processor.h>

#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <libgen.h>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

void endProgram(char *name)
{
  cout << basename(name) << ": process a bilingual stream with a lexical rule transducer" << endl;
  cout << "USAGE: " << basename(name) << "[ -t | -d ] fst_file [input_file [output_file]]" << endl;
#if HAVE_GETOPT_LONG
  cout << "  -t, --trace:         trace the rules which have been applied" << endl;
  cout << "  -d, --debug:         print out information about how the rules are run" << endl;
#else
  cout << "  -t:         trace the rules which have been applied" << endl;
  cout << "  -d:         print out information about how the rules are run" << endl;
#endif
  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
  LRXProcessor lrxp;

#if HAVE_GETOPT_LONG
  static struct option long_options[]=
    {
      {"trace",        0, 0, 't'}
      {"debug",        0, 0, 'd'}
    };
#endif

  while(true)
  {
#if HAVE_GETOPT_LONG
    int option_index;
    int c = getopt_long(argc, argv, "td", long_options, &option_index);
#else
    int c = getopt(argc, argv, "td");
#endif

    if(c == -1)
    {
      break;
    }

    switch(c)
    {
    case 't':
      lrxp.setTraceMode(true);
      break;
    case 'd':
      lrxp.setDebugMode(true);
      break;
    default:
      endProgram(argv[0]);
      break;
    }
  }

  FILE *input = stdin, *output = stdout;
  LtLocale::tryToSetLocale();

  if(optind == (argc - 3))
  {
    FILE *in = fopen(argv[optind], "rb");
    if(in == NULL || ferror(in))
    {
      endProgram(argv[0]);
    }

    input = fopen(argv[optind+1], "rb");
    if(input == NULL || ferror(input))
    {
      endProgram(argv[0]);
    }

    output= fopen(argv[optind+2], "wb");
    if(output == NULL || ferror(output))
    {
      endProgram(argv[0]);
    }

    lrxp.load(in);
    fclose(in);
  }
  else if(optind == (argc -2))
  {
    FILE *in = fopen(argv[optind], "rb");
    if(in == NULL || ferror(in))
    {
      endProgram(argv[0]);
    }

    input = fopen(argv[optind+1], "rb");
    if(input == NULL || ferror(input))
    {
      endProgram(argv[0]);
    }

    lrxp.load(in);
    fclose(in);
  }
  else if(optind == (argc - 1))
  {
    FILE *in = fopen(argv[optind], "rb");
    if(in == NULL || ferror(in))
    {
      endProgram(argv[0]);
    }
    lrxp.load(in);
    fclose(in);
  }
  else
  {
    endProgram(argv[0]);
  }

#ifdef _MSC_VER
        _setmode(_fileno(input), _O_U8TEXT);
        _setmode(_fileno(output), _O_U8TEXT);
#endif

  lrxp.init();
  lrxp.process(input, output);

  fclose(input);
  fclose(output);
  return EXIT_SUCCESS;
}
