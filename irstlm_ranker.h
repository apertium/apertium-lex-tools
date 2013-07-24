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

#include <limits>

#include "lmContainer.h"
#include "lmmacro.h"
#include "lmtable.h"

class lmtable;  // irst lm table
class lmmacro;  // irst lm for macro tags

class IrstlmRanker {
private:

	double lowerBound;
	double upperBound;	
	double filter;

    vector<double> scores;
    vector<string> batch;

	int current_line;
    int maxlineno;
	int lineno;
	int sublineno;

    double current_max;

protected:
	lmContainer        *m_lmtb;

	int            m_unknownId;
	int            m_lmtb_size;          // max ngram stored in the table
	int            m_lmtb_dub;           // dictionary upperbound

	float          m_weight; // scoring weight.
	std::string    m_filePath; // for debugging purposes.
	size_t         m_nGramOrder; // max n-gram length contained in this LM.

	vector<std::string> parseLine(std::string);
	void printScores(vector<string> batch, 
					 vector<double> scores, double total);

	void reset();

	std::string trim(const std::string& o);
	bool load(const std::string &filePath, float weight);
	double score(const std::string &frame, double &pp);	

public:
	IrstlmRanker(const string &filePath, vector<double> params);	
	~IrstlmRanker();
	
	int standard();
	int fractional();
	int max();
	int totals();
};

#endif
