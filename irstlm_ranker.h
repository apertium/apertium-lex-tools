#ifndef IRSTLM_RANKER_H
#define IRSTLM_RANKER_H

#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <getopt.h>
#include <clocale>
#include <vector>
#include <deque>
#include <map>

#include "lmContainer.h"
#include "lmmacro.h"
#include "lmtable.h"

class lmtable;  // irst lm table
class lmmacro;  // irst lm for macro tags

class IrstlmRanker {
private:

	string current_line;
    double current_max;
    map<string, double> scores;
    map<string, string> batch;
    string maxlineno;
	string lineno;
	string sublineno;
    double total;

protected:
	lmContainer        *m_lmtb;

	int            m_unknownId;
	int            m_lmtb_size;          // max ngram stored in the table
	int            m_lmtb_dub;           // dictionary upperbound

	float          m_weight; // scoring weight.
	std::string         m_filePath; // for debugging purposes.
	size_t         m_nGramOrder; // max n-gram length contained in this LM.

	vector<std::string> parseLine(std::string);
	void printScores(map<string, string> batch, 
					 map<string, double> scores);

	void reset();

	std::string trim(const std::string& o);
	bool load(const std::string &filePath, float weight);
	double score(const std::string &frame, double &pp);	

public:
	IrstlmRanker(const string &filePath);	
	~IrstlmRanker();
	
	int standard();
	int fractional();
	int max();
	int totals();
};

#endif
