#ifndef IRSTLM_RANKER_MAX_H
#define IRSTLM_RANKER_MAX_H

#include "irstlm_ranker.h"
#include <map>
class IrstlmRankerMax : public IrstlmRanker {

public:
	IrstlmRankerMax(const std::string &filePath);	
	~IrstlmRankerMax();

	virtual int main();
};

#endif
