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

#include <lrx_compiler.h>

#define PACKAGE_VERSION "0.1.0"

using namespace std;

void endProgram(char *name)
{
  if(name != NULL)
  {
    cout << basename(name) << " v" << PACKAGE_VERSION <<": build a selection transducer from a ruleset" << endl;
    cout << "USAGE: " << basename(name) << " rule_file output_file" << endl;
  }
  exit(EXIT_FAILURE);
}

int main (int argc, char **argv)
{
  LRXCompiler compiler;

  if(argc < 3) 
  {
    endProgram(argv[0]);
  }

  compiler.parse(argv[1]);

  FILE *output = fopen(argv[2], "wb");

  compiler.write(output);

  return 0;
}
