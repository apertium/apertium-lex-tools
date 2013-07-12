#include "TaggerOutputProcessor.h"

TaggerOutputProcessor::TaggerOutputProcessor() {
	LtLocale::tryToSetLocale();
}

TaggerOutputProcessor::~TaggerOutputProcessor() {

}

void TaggerOutputProcessor::printBiltransSentence(vector<BiltransToken> s) {
	for(int i = 0; i < s.size(); i++) {
		wcout << s[i].toString(true);
		if (i != s.size() - 1) {
			wcout << L" ";
		}
	}
	wcout << endl;
}

int TaggerOutputProcessor::find(vector<wstring> xs, wstring x) {
	for (int i = 0; i < xs.size(); i++) {
		if (xs[i] == x)
			return i;
	}
	return -1;

}

FSTProcessor TaggerOutputProcessor::loadBilingual(string path) {
	FSTProcessor bilingual;

	FILE *f_bin = fopen(path.c_str(), "r");

	bilingual.load(f_bin);

	fclose(f_bin);
	bilingual.initBiltrans();
	return bilingual;
}

BiltransToken TaggerOutputProcessor::parseBiltransToken(wstring bt) {

	BiltransToken token;
	vector<wstring> tokens = wsplit(bt, L'/');
	
	token.sourceToken = parseTaggerToken(tokens[0]);

	for (int i = 1; i < tokens.size(); i++) {
		token.targetTokens.push_back(parseTaggerToken(tokens[i]));
	}
	return token;

}

TaggerToken TaggerOutputProcessor::parseTaggerToken(wstring str) {
	TaggerToken token;
	int state = 0; // lemma;
	wstring buffer;
	wchar_t c;
	for (int i = 0; i < str.size(); i++) {
		c = str[i];
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

vector<wstring> TaggerOutputProcessor::parseTags(wstring token) {
	int state = 0; // outside
	vector<wstring> tags;
	wstring buffer;
	for(int i = 0; i < token.size(); i++) {
		wchar_t c = token[i];
		if (state == 0) {
			if (c == '<') {
				state = 1;
			}
		} else if (state == 1) {
			if (c == '>') {
				tags.push_back(buffer);
				buffer = L"";
				state = 0;
			} else {
				buffer += c;
			}
		} 
	}
	return tags;
}

vector<wstring> TaggerOutputProcessor::wsplit(wstring wstr, wchar_t delim) {
	vector<wstring> tokens;
	wstring buffer;

	for(int i = 0; i < wstr.size(); i++) {
		if(wstr[i] == delim) {
			tokens.push_back(buffer);
			buffer = L"";
		} else {
			buffer += wstr[i];
		}
	}
	if(buffer != L"") {
		tokens.push_back(buffer);
	}
	return tokens;
}




wstring TaggerOutputProcessor::getLemma(wstring token) {
	wstring buffer;
	for(int i = 0; i < token.size(); i++) {
		if(token[i] != '<') {
			buffer += token[i];
		} else {
			break;
		}
	}
	return buffer;
}

vector<vector<TaggerToken> > TaggerOutputProcessor::parseTaggerOutput() {

	wstring buffer;
	vector<TaggerToken> sentence;
	vector<vector<TaggerToken> > sentences; 
	bool escaped = false;
	int state = 0; // outside
	wchar_t c;
	while(c = fgetwc(stdin)) {
		if (c == -1) {
			break;
		}
		if(c == L'\n') {
			if (buffer.size() > 0) {
				sentence.push_back(parseTaggerToken(buffer));
			}
			if (sentence.size() > 0) {
				sentences.push_back(sentence);
			}
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
				buffer = L"";
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
	return sentences;
}
