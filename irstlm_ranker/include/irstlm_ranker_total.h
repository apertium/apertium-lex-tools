#ifndef IRSTLM_RANKER_TOTAL_H
#define IRSTLM_RANKER_TOTAL_H

#include "irstlm_ranker.h"

class IrstlmRankerTotal : public IrstlmRanker {

public:
	IrstlmRankerTotal(const std::string &filePath);	
	~IrstlmRankerTotal();

	virtual int main();
};

#endif
