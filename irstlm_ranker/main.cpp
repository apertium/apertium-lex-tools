#include "include/irstlm_ranker_s.h"
#include "include/irstlm_ranker_frac.h"

#include "include/irstlm_ranker_total.h"
#include "include/irstlm_ranker_max.h"

using namespace std;


void printError(char* name) {
wcerr<<"Error: Wrong number of parameters"<<endl;
    wcerr<<"Usage: "<<name<<" lm_file mode"<<endl;
	wcerr<<"modes:" << endl;
	wcerr<<"\t -s | --standard"<<endl;
	wcerr<<"\t -f | --fractional-counts"<<endl;
	wcerr<<"\t -t | --total-counts"<<endl;
	wcerr<<"\t -m | --max-count"<<endl;
    exit(EXIT_FAILURE);
}
int main(int argc, char ** argv) {

// Is this really necessary?
  if(setlocale(LC_CTYPE, "") == NULL) {
    wcerr << L"Warning: unsupported locale, fallback to \"C\"" << endl;
    setlocale(LC_ALL, "C");
  }

  if (argc != 3) {
    printError(argv[0]);
  }
//	IrstlmRanker *irstlm_ranker;
	string mode(argv[2]);
	if (mode == "--standard" || mode == "-s") {
		IrstlmRankerStandard irstlm_ranker(argv[1]);
		irstlm_ranker.main();
	} else if (mode == "--fractional-counts" || mode == "-f") {
		IrstlmRankerFractional irstlm_ranker(argv[1]);
		irstlm_ranker.main();
		//irstlm_ranker = new IrstlmRankerFractional(argv[1]);
	} else if (mode == "--total-counts" || mode == "-t") {
		IrstlmRankerTotal irstlm_ranker(argv[1]);
		irstlm_ranker.main();
	} else if (mode == "--max-count" || mode == "-m") {
		IrstlmRankerMax irstlm_ranker(argv[1]);
		irstlm_ranker.main();
	} else {
		printError(argv[0]);
	}
	
//	delete irstlm_ranker;		
//	return irstlm_ranker->main();


}
