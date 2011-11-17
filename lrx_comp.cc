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
