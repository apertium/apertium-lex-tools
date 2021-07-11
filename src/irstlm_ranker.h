#ifndef IRSTLM_RANKER_H
#define IRSTLM_RANKER_H

#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <getopt.h>
#include <clocale>
#include <vector>
#include <deque>
#include <map>
#include <fstream>
#include <istream>
#include <limits>

#include <irstlm/lmContainer.h>
#include <irstlm/lmmacro.h>
#include <irstlm/lmtable.h>

class lmtable;  // irst lm table
class lmmacro;  // irst lm for macro tags

class IrstlmRanker {
private:

	double probMassThr;

	std::vector<long double> logScores;
	std::vector<long double> probs;
	std::vector<int> sortedIndex;

	std::vector<string> batch;

	int current_line;
	int maxlineno;
	int lineno;
	int sublineno;

	long double norm;
	long double current_max;

protected:
	irstlm::lmContainer        *m_lmtb;

	int            m_unknownId;
	int            m_lmtb_size;          // max ngram stored in the table
	int            m_lmtb_dub;           // dictionary upperbound

	float          m_weight; // scoring weight.
	std::string    m_filePath; // for debugging purposes.
	size_t         m_nGramOrder; // max n-gram length contained in this LM.

	std::vector<std::string> parseLine(std::string);
	void printScores(std::vector<long double> scores);
	void insertSortedIndex(long double prob);

	void reset();
	void normalizeProbabilities();

	std::ifstream tmtrans;

	std::string trim(const std::string& o);
	bool load(const std::string &filePath, float weight);
	double score(const std::string &frame, double &pp);

public:
	IrstlmRanker(const std::string &filePath, char *mtransPath, std::vector<double> params);
	~IrstlmRanker();

	int standard();
	int fractional();
};

#endif
