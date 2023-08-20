#include "BiltransWithoutQueue.h"
#include <i18n.h>

int main(int argc, char** argv) {

	if (argc != 2 && argc != 3) {
		cout << I18n(APLT_I18N_DATA, "aplt").format("biltrans_without_queue_desc");
			 << endl;
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
