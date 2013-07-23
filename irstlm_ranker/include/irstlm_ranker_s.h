#ifndef IRSTLM_RANKER_S_H
#define IRSTLM_RANKER_S_H

#include "irstlm_ranker.h"

class IrstlmRankerStandard : public IrstlmRanker {

public:
	IrstlmRankerStandard(const std::string &filePath);	
	~IrstlmRankerStandard();

	virtual int main();
};

#endif
