/*
 * Copyright (C) 2011--2012 Universitat d'Alacant
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

#include <lrx_compiler.h>
#include <cstring>
#include <iostream>
#include <libgen.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/i18n.h>

using namespace std;

void endProgram(char *name)
{
  if(name != NULL)
  {
    cout << I18n(ALX_I18N_DATA, "alx").format("lrx_comp_desc", {"program", "version"},
                {basename(name), PACKAGE_VERSION})
         << endl;
  }
  exit(EXIT_FAILURE);
}

int main (int argc, char **argv)
{
  LtLocale::tryToSetLocale();

  LRXCompiler compiler;

  if(argc != 3 && argc != 4)
  {
    endProgram(argv[0]);
  }

  if(argc == 3)
  {
    compiler.parse(argv[1]);
    FILE *output = fopen(argv[2], "wb");
    compiler.write(output);
  }
  else if(argc == 4)
  {
    if(strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--print-transducer") == 0)
    {
      compiler.setOutputGraph(true);
    }
    if(strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0)
    {
      compiler.setOutputGraph(true);
      compiler.setDebugMode(true);
    }

    compiler.parse(argv[2]);
    FILE *output = fopen(argv[3], "wb");
    compiler.write(output);
  }
}
