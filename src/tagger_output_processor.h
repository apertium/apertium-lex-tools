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
#include <apertium/tsx_reader.h>
#include <apertium/string_utils.h>

using namespace std;

class TaggerToken {
public:
  UString lemma;
  vector<UString> tags;
  UString toString(bool delimiters) {
    UString out;
    if (delimiters) {
      out += '^';
    }
    out.append(lemma);
    for (auto& tag : tags) {
      out += '<';
      out.append(tag);
      out += '>';
    }
    if (delimiters) {
      out += '$';
    }
    return out;
  }
};

class TaggerOutputProcessor {
protected:
	int sn;

	vector<UString> parseTags(UString token);
	vector<UString> wsplit(UString wstr, wchar_t delim);
	TaggerToken parseTaggerToken(UString buffer);

	int find(vector<UString> xs, UString x);
	UString getLemma(UString token);

	virtual void processSentence(vector<TaggerToken>) =0;
public:
	TaggerOutputProcessor();
	~TaggerOutputProcessor();

	void processTaggerOutput(bool nullFlush=false);

};

#endif
