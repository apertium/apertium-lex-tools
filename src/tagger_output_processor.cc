#include "tagger_output_processor.h"

TaggerOutputProcessor::TaggerOutputProcessor() {
	sn = 0;
	LtLocale::tryToSetLocale();
}

TaggerOutputProcessor::~TaggerOutputProcessor() {

}

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
		if(c == L'<' && state == 0) {
			state = 1;
			token.lemma = buffer;
			buffer.clear();
		}

		if (c == L'>') {
			token.tags.push_back(buffer);
			buffer.clear();
		} else if (c != L'<') {
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

vector<UString> TaggerOutputProcessor::wsplit(UString wstr, wchar_t delim) {
	vector<UString> tokens;
	UString buffer;

	for(size_t i = 0; i < wstr.size(); ++i) {
		if(wstr[i] == delim && (i == 0 || wstr[i-1] != L'\\')) {
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
	UString buffer;
	vector<TaggerToken> sentence;
	bool escaped = false;
	int state = 0; // outside
	wchar_t c;
	while((c = fgetwc(stdin))) {
		if (c == -1) {
			break;
		}

		if (nullFlush && c == L'\0') {
		  processSentence(sentence);
		  sentence.clear();
		  buffer.clear();
		}

		if(c == L'\n') {
			processSentence(sentence);
			sentence.clear();
			buffer.clear();
		}
		if (state == 0) {
			if (c == '^' && !escaped) {
				state = 1; // inside
			} else if (c == '\\' && !escaped) {
				escaped = true;
			} else {
				escaped = false;
			}
		} else if (state == 1) {
			if(c == L'$' && !escaped) {
				sentence.push_back(parseTaggerToken(buffer));
				buffer.clear();
				state = 0;
			} else if (c == '\\' && !escaped) {
				escaped = true;
				buffer += c;
			} else {
				buffer += c;
				escaped = false;
			}
		}
	}
}
