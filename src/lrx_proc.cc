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
#include <lrx_processor.h>

#include <lttoolbox/lt_locale.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/file_utils.h>

using namespace std;

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  CLI cli("process a bilingual stream with a lexical rule transducer", PACKAGE_VERSION);
  cli.add_bool_arg('t', "trace", "trace rules which have been applied");
  cli.add_bool_arg('d', "debug", "print out information about which rules are run");
  cli.add_bool_arg('z', "null-flush", "flush on the null character");
  cli.add_bool_arg('m', "max-ent", "no-op (retained for backwards compatibility)");
  cli.add_bool_arg('h', "help", "print this message and exit");
  cli.add_file_arg("fst_file", false);
  cli.add_file_arg("input_file", true);
  cli.add_file_arg("output_file", true);
  cli.parse_args(argc, argv);

  LRXProcessor lrxp;

  lrxp.setNullFlush(cli.get_bools()["null-flush"]);
  lrxp.setTraceMode(cli.get_bools()["trace"]);
  lrxp.setDebugMode(cli.get_bools()["debug"]);

  FILE* in = openInBinFile(cli.get_files()[0]);
  lrxp.load(in);
  fclose(in);

  InputFile input;
  if (!cli.get_files()[1].empty()) {
    input.open_or_exit(cli.get_files()[1].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[2]);

  lrxp.init();
  lrxp.process(input, output);
  u_fclose(output);
  return EXIT_SUCCESS;
}
