#include "../include/irstlm_ranker_total.h"
using namespace std;

IrstlmRankerTotal::IrstlmRankerTotal(const string &filePath) :
    IrstlmRanker(filePath) {

}

IrstlmRankerTotal::~IrstlmRankerTotal() {

}

int IrstlmRankerTotal::main() {
    double total = 0.0;
  int numlines = 0;
  while (!cin.eof()) {
    string line;
    getline(cin, line);
    cerr << "@: " << line << endl;
    if (line.length()>0) {
      double pp;
      double log_prob = score("<s> " + line + " </s>", pp);

       
      cout<< log_prob<< "\t||\t" << line <<endl;

      total += log_prob ; 
      numlines++;
    }
  }
   cout << "log_total: " << total << endl;
   cout << "prob_total: " << exp10(total) << endl;
   cout << "log_avg: " << total/numlines << endl;
   cout << "prob_avg: " << exp10(total)/numlines << endl;

  return EXIT_SUCCESS;

}
