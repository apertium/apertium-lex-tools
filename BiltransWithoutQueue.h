#ifndef BILTRANS_WITHOUT_QUEUE
#define BILTRANS_WITHOUT_QUEUE

#include "TaggerOutputProcessor.h"

class BiltransWithoutQueue : public TaggerOutputProcessor {
private:
	FSTProcessor bilingual;
	string path;
	bool trimmed;
	bool isPosAmbig(BiltransToken token);
	
	BiltransToken getTrimmedToken(BiltransToken token);
	void biltransToMultitrans(int sn, int &tn, int idx, 
			vector<BiltransToken> s, wstring buffer);
	
public:
	BiltransWithoutQueue(string path, bool trimmed);
	~BiltransWithoutQueue();

	int calculateFertility(vector<BiltransToken> sent);
	void processTaggerOutput();
};

#endif
