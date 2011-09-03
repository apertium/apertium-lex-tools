#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <libgen.h>

#include <lttoolbox/fst_processor.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>

#define PACKAGE_VERSION "0.1.0"

using namespace std;

FSTProcessor fstp;

int main(int argc, char **argv)
{
  LtLocale::tryToSetLocale();

  if(argc < 2) 
  { 
    cout << basename(argv[0]) << " v" << PACKAGE_VERSION <<": assign default lexical selection values" << endl;
    cout << "USAGE: " << basename(argv[0]) << " lex_file " << endl;
    exit(-1);
  }

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

  wstring input = L"^patró<n><m><pl>$";
  wstring trad = fstp.biltrans(input);

  wcout << input << L" --> " << trad << endl;

  return 0;
}
