#include "../include/irstlm_ranker_s.h"
using namespace std;

IrstlmRankerStandard::IrstlmRankerStandard(const string &filePath) :
IrstlmRanker(filePath) {

}

IrstlmRankerStandard::~IrstlmRankerStandard() {

}

int IrstlmRankerStandard::main() {
	while (!cin.eof()) {
    string line;
    getline(cin, line);
    if (line.length()>0) {
      double pp;
      double log_prob = score("<s> " + line + " </s>", pp);

       
      cout<< log_prob<< "\t||\t" << line <<endl;
    }
  }

  return EXIT_SUCCESS;
}
