#include <stdio.h>
#include <string>
#include <iostream>

#include <lttoolbox/fst_processor.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/ltstr.h>

using namespace std;

FSTProcessor fstp;

int main(int argc, char **argv)
{
	if(argc < 2) { 
		wcout << L"Please specify a transducer" << endl;
		exit(-1);
	}

    LtLocale::tryToSetLocale();
	FILE *t_rl = fopen(argv[1], "r");

	fstp.load(t_rl);
	fclose(t_rl);
	fstp.initBiltrans();

	wstring input;
	while (cin >> input) {
		wstring trad = fstp.biltrans(input);
		wcout << trad << endl;
	}

	return 0;
}
