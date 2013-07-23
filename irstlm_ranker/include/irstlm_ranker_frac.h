#ifndef IRSTLM_RANKER_FRAC_H
#define IRSTLM_RANKER_FRAC_H

#include "irstlm_ranker.h"
#include <map>
class IrstlmRankerFractional : public IrstlmRanker {

public:
	IrstlmRankerFractional(const std::string &filePath);	
	~IrstlmRankerFractional();

	virtual int main();
};

#endif
