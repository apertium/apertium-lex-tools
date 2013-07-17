#ifndef BILTRANS_WITHOUT_QUEUE
#define BILTRANS_WITHOUT_QUEUE

#include "TaggerOutputProcessor.h"

class BiltransToken {
public:
	TaggerToken sourceToken;	
	vector<TaggerToken> targetTokens;
	
	wstring toString(bool delimiter) {
		wstring out = sourceToken.toString(false);
		for(int i = 0; i < targetTokens.size(); i++) {
			out += L'/' + targetTokens[i].toString(false);
		}
		if (delimiter) {
			out = L"^" + out + L"$";
		}
		return out;
	}
};

class Multitrans : public TaggerOutputProcessor {
private:
	FSTProcessor bilingual;
	string path;

	bool trimmed;
	bool filter;
	string mode;

	bool isPosAmbig(BiltransToken token);	
	
	BiltransToken getTrimmedToken(wstring str);
	BiltransToken getFullToken(wstring str);
	
	BiltransToken parseBiltransToken(wstring bt);
	
	void processSentence(vector<TaggerToken> s);

	void printBiltransSentence(int i, vector<BiltransToken> s);

	void printTaggerOutput(vector<BiltransToken> s);
	
	void biltransToMultitrans(int sn, int &tn, int idx, 
			vector<BiltransToken> s, wstring buffer);

	

public:
	Multitrans(string path, string mode, bool trimmed, bool filter);
	~Multitrans();

	int calculateFertility(vector<BiltransToken> sent);

};

#endif

