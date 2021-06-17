#include "BiltransWithoutQueue.h"

int main(int argc, char** argv) {

	if (argc != 2 && argc != 3) {
		cout << "Usage: " << argv[0];
		cout << "<path to a binary bilingual transducer> [--trimmed | -t]" << endl;
		exit(1);
	}
	string path(argv[1]);
	bool trimmed = false;

	if (argc == 3) {
		string param(argv[2]);
		trimmed = (param == "--trimmed" || param == "-t");
	}
	BiltransWithoutQueue bwq(path, trimmed);
	bwq.processTaggerOutput();
}
