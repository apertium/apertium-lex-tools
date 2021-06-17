#include "multi_translator.h"
#include <lttoolbox/lt_locale.h>

bool trim = false;
bool filter = false;
bool number_lines = false;
bool null_flush = false;

string path;
string mode;

void printError(char *name) {
    cout << "Usage: " << name << " ";
    cout << "<mode> [options] <path to a binary bilingual transducer>" << endl;
    cout << "Modes: " << endl;
    cout << "  --biltrans           | -b" << endl;
    cout << "  --multitrans         | -m" << endl;
    cout << "  --trim-tagger-output | -p" << endl;

    cout << "Options: " << endl;
    cout << "  --filter-lines | -f" << endl;
    cout << "  --trim-lines   | -t" << endl;
    cout << "  --number-lines | -n" << endl;
    cout << "  --null-flush   | -z" << endl;
}

void parseArguments(int argc, char **argv) {
    if (argc < 3 || argc > 7) {
      printError(argv[0]);
      exit(1);
    }

    mode = argv[1];
    if (mode == "--biltrans") {
      mode = "-b";
    } else if (mode == "--multitrans") {
      mode = "-m";
    } else if (mode == "--tagger-output") {
      mode = "-p";
    }

    if (mode != "-b" && mode != "-m" && mode != "-p") {
      printError(argv[0]);
      exit(1);
    }

    for (int i=2; i < argc; ++i) {
      if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trim-lines") == 0) {
        trim = true;
      } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--filter-lines") == 0) {
        filter = true;
      } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number-lines") == 0) {
        number_lines = true;
      } else if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--null-flush") == 0) {
        null_flush = true;
      }
    }

    path = argv[argc-1];
}

int main(int argc, char** argv) {
  LtLocale::tryToSetLocale();
    parseArguments(argc, argv);

    MultiTranslator mt(path, mode, trim, filter, number_lines);
    if (null_flush) {
      mt.processTaggerOutput(true);
    } else {
      mt.processTaggerOutput();
    }
}
