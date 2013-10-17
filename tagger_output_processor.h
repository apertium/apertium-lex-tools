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
		for(unsigned int i = 0; i < tags.size(); i++) {
			out += L"<" + tags[i] + L">";
		}
		if (delimiters) {
			out = L"^" + out + L"$";
		}
		return out;
	}
};

class TaggerOutputProcessor {
protected:
	int sn;

	vector<wstring> parseTags(wstring token);
	vector<wstring> wsplit(wstring wstr, wchar_t delim);
	TaggerToken parseTaggerToken(wstring buffer);
	
	int find(vector<wstring> xs, wstring x);
	wstring getLemma(wstring token);

	virtual void processSentence(vector<TaggerToken>) =0;
public:
	TaggerOutputProcessor();
	~TaggerOutputProcessor();
	
	void processTaggerOutput();

};

#endif
