#include "tagger_output_processor.h"
#include <lttoolbox/string_utils.h>
#include <lttoolbox/input_file.h>

int TaggerOutputProcessor::find(vector<UString> xs, UString x) {
	for (size_t i = 0; i < xs.size(); ++i) {
		if (xs[i] == x)
			return i;
	}
	return -1;
}

TaggerToken TaggerOutputProcessor::parseTaggerToken(UString str) {
	TaggerToken token;
	int state = 0; // lemma;
	UString buffer;
	for (auto& c : str) {
		if(c == '<' && state == 0) {
			state = 1;
			token.lemma = buffer;
			buffer.clear();
		}

		if (c == '>') {
			token.tags.push_back(buffer);
			buffer.clear();
		} else if (c != '<') {
			buffer += c;
		}
	}
	if(state == 0) {
		token.lemma = buffer;
	}
	return token;
}

vector<UString> TaggerOutputProcessor::parseTags(UString token) {
	int state = 0; // outside
	vector<UString> tags;
	UString buffer;
	for (auto& c : token) {
		if (state == 0) {
			if (c == '<') {
				state = 1;
			}
		} else if (state == 1) {
			if (c == '>') {
				tags.push_back(buffer);
				buffer.clear();
				state = 0;
			} else {
				buffer += c;
			}
		}
	}
	return tags;
}

vector<UString> TaggerOutputProcessor::wsplit(UString wstr, UChar delim) {
	vector<UString> tokens;
	UString buffer;

	for(size_t i = 0; i < wstr.size(); ++i) {
		if(wstr[i] == delim && (i == 0 || wstr[i-1] != '\\')) {
			tokens.push_back(buffer);
			buffer.clear();
		} else {
			buffer += wstr[i];
		}
	}
	if(!buffer.empty()) {
		tokens.push_back(buffer);
	}
	return tokens;
}

UString TaggerOutputProcessor::getLemma(UString token) {
	UString buffer;
	for (auto& c : token) {
		if(c != '<') {
			buffer += c;
		} else {
			break;
		}
	}
	return buffer;
}

void TaggerOutputProcessor::processTaggerOutput(bool nullFlush) {
	vector<TaggerToken> sentence;
	UChar32 c;
  InputFile in;
	while (!in.eof()) {
    c = in.get();

		if ((c == '\n') || (nullFlush && c == '\0')) {
		  processSentence(sentence);
		  sentence.clear();
		} else if (c == '\\') {
      in.get();
    } else if (c == '^') {
      sentence.push_back(parseTaggerToken(in.readBlock('^', '$')));
		}
	}
}
