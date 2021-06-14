#ifndef TAGGER_OUTPUT_PROCESSOR
#define TAGGER_OUTPUT_PROCESSOR

#include <stdio.h>
#include <lttoolbox/ustring.h>

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
	int sn = 0;

	vector<UString> parseTags(UString token);
	vector<UString> wsplit(UString wstr, UChar delim);
	TaggerToken parseTaggerToken(UString buffer);

	int find(vector<UString> xs, UString x);
	UString getLemma(UString token);

	virtual void processSentence(vector<TaggerToken>) =0;
public:
	void processTaggerOutput(bool nullFlush=false);
};

#endif
