#ifndef TAGGER_OUTPUT_PROCESSOR
#define TAGGER_OUTPUT_PROCESSOR

#include <stdio.h>
#include <string>
#include <iostream>

#include <lttoolbox/fst_processor.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/ltstr.h>

#include <cwchar>
#include <set>
#include <apertium/tagger.h>
#include <apertium/tsx_reader.h>
#include <apertium/string_utils.h>

using namespace std;

class TaggerToken {
public:
	wstring lemma;
	vector<wstring> tags;
	wstring toString(bool delimiters) {
		wstring out = lemma;
		for(int i = 0; i < tags.size(); i++) {
			out += L"<" + tags[i] + L">";
		}
		if (delimiters) {
			out = L"^" + out + L"$";
		}
		return out;
	}
};

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

class TaggerOutputProcessor {
protected:
	vector<vector<TaggerToken> > parseTaggerOutput();
	vector<wstring> parseTags(wstring token);
	vector<wstring> wsplit(wstring wstr, wchar_t delim);
	TaggerToken parseTaggerToken(wstring buffer);
	BiltransToken parseBiltransToken(wstring bt);

	void printBiltransSentence(vector<BiltransToken> s);
	
	int find(vector<wstring> xs, wstring x);
	wstring getLemma(wstring token);

	FSTProcessor loadBilingual(string path);
public:
	TaggerOutputProcessor();
	~TaggerOutputProcessor();
	
	virtual void processTaggerOutput() =0;

};

#endif
