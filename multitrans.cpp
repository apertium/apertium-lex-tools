#include "Multitrans.h"


void printError(char *name) {
	wcout << "Usage: " << name << " ";
	wcout << "<path to a binary bilingual transducer> <mode> [--trimmed | -t]" << endl;
	wcout << "Modes: " << endl;
	wcout << "  --biltrans | -b" << endl;
	wcout << "  --multitrans | -m" << endl;
}

int main(int argc, char** argv) {

	if (argc != 3 && argc != 4) {
		printError(argv[0]);
		exit(1);
	}
	string path(argv[1]);
	string mode(argv[2]);
	
	if ( mode == "--biltrans") {
		mode = "-b";
	} else if (mode == "--multitrans") {
		mode = "-m";
	} 
	if(mode != "-b" && mode != "-m") {
		printError(argv[0]);
		exit(1);
	}
	bool trimmed = false;

	if (argc == 4) {
		string option(argv[3]);
		trimmed = (option == "--trimmed" || option == "-t");
	}
	Multitrans mt(path, mode, trimmed);
	mt.processTaggerOutput();
}
