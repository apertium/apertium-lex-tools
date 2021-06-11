// -*- mode: c++ -*-

#ifndef BILTRANS_WITHOUT_QUEUE
#define BILTRANS_WITHOUT_QUEUE

#include "tagger_output_processor.h"

class BiltransToken {
public:
  TaggerToken sourceToken;
  vector<TaggerToken> targetTokens;
  UString blanks;
  
  bool isEOF;
  
  BiltransToken() {
    isEOF = false;
  }

  UString toString(bool delimiter) {
    UString out;
    if (delimiter) {
      out += '^';
    }
    out.append(sourceToken.toString(false));
    for (auto& tok : targetTokens) {
      out += '/';
      out.append(tok.toString(false));
    }
    if (delimiter) {
      out += '$';
    }
    return out;
  }
};

class MultiTranslator : public TaggerOutputProcessor {
private:
	FSTProcessor bilingual;
	map<UString, UString> f_cache;
	map<UString, UString> t_cache;
	string path;

	bool trimmed;
	bool filter;
	bool number_lines;

	string mode;

	bool isPosAmbig(BiltransToken token);

	BiltransToken getTrimmedToken(UString str);
	BiltransToken getFullToken(UString str);

	BiltransToken parseBiltransToken(UString bt);

	void processSentence(vector<TaggerToken> s);

	void printBiltransSentence(int i, vector<BiltransToken> s);

	void printTaggerOutput(int i, vector<BiltransToken> s);

	void biltransToMultiTranslator(int sn, int &tn, unsigned int idx,
			vector<BiltransToken> s, UString buffer);



public:
	MultiTranslator(string path, string mode, bool trimmed, bool filter, bool number_lines);
	~MultiTranslator();

	int calculateFertility(vector<BiltransToken> sent);

};

#endif

