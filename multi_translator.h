#ifndef BILTRANS_WITHOUT_QUEUE
#define BILTRANS_WITHOUT_QUEUE

#include "tagger_output_processor.h"

class BiltransToken {
public:
	TaggerToken sourceToken;	
	vector<TaggerToken> targetTokens;
	wstring blanks;

	bool isEOF;
	
	BiltransToken() {
		isEOF = false;
	}

	wstring toString(bool delimiter) {
		wstring out = sourceToken.toString(false);
		for(unsigned int i = 0; i < targetTokens.size(); i++) {
			out += L'/' + targetTokens[i].toString(false);
		}
		if (delimiter) {
			out = L"^" + out + L"$";
		}
		return out;
	}
};

class MultiTranslator : public TaggerOutputProcessor {
private:
	FSTProcessor bilingual;
	string path;

	bool trimmed;
	bool filter;
	bool number_lines;

	string mode;

	bool isPosAmbig(BiltransToken token);	
	
	BiltransToken getTrimmedToken(wstring str);
	BiltransToken getFullToken(wstring str);
	
	BiltransToken parseBiltransToken(wstring bt);
	
	void processSentence(vector<TaggerToken> s);

	void printBiltransSentence(int i, vector<BiltransToken> s);

	void printTaggerOutput(int i, vector<BiltransToken> s);
	
	void biltransToMultiTranslator(int sn, int &tn, unsigned int idx, 
			vector<BiltransToken> s, wstring buffer);

	

public:
	MultiTranslator(string path, string mode, bool trimmed, bool filter, bool number_lines);
	~MultiTranslator();

	int calculateFertility(vector<BiltransToken> sent);

};

#endif

