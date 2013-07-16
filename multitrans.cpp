#include "Multitrans.h"

bool trim = false;
bool filter = false;

string path;
string mode;

void printError(char *name) {
	wcout << "Usage: " << name << " ";
	wcout << "<path to a binary bilingual transducer> <mode> [options]" << endl;
	wcout << "Modes: " << endl;
	wcout << "  --biltrans | -b" << endl;
	wcout << "  --multitrans | -m" << endl;
	wcout << "  --trim-tagger-output | -p" << endl;

	wcout << "Options: " << endl;
	wcout << "  --filter-lines | -f" << endl;
	wcout << "  --trim-lines | -t" << endl;

}

void parseArguments(int argc, char **argv) {
	if (argc < 3 || argc > 5) {
		printError(argv[0]);
		exit(1);
	}
	path = argv[1];
	mode = argv[2];
	
	if ( mode == "--biltrans") {
		mode = "-b";
	} else if (mode == "--multitrans") {
		mode = "-m";
	} else if (mode == "--tagger-output") {
		mode = "-p";
	}
	if(mode != "-b" && mode != "-m" && mode != "-p") {
		printError(argv[0]);
		exit(1);
	}

	for(int i = 2; i < argc; i++) {
		if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "-trim-lines") == 0) {
			trim = true;
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "-filter-lines") == 0) {
			filter = true;
		}
	}
}


int main(int argc, char** argv) {

	parseArguments(argc, argv);

	Multitrans mt(path, mode, trim, filter);
	mt.processTaggerOutput();
}
