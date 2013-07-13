#include "Multitrans.h"

Multitrans::Multitrans(string path, string mode, bool trimmed) {
	this->trimmed = trimmed;
	this->path = path;
	this->mode = mode;

	FILE *f_bin = fopen(path.c_str(), "r");
	bilingual.load(f_bin);

	fclose(f_bin);
	bilingual.initBiltrans();
}
Multitrans::~Multitrans() {}

int Multitrans::calculateFertility(vector<BiltransToken> sent) {
	int fertility = 1;
	for (int i = 0; i < sent.size(); i++) {
		fertility *= sent[i].targetTokens.size();
	}
	return fertility;
}


BiltransToken Multitrans::parseBiltransToken(wstring bt) {

	BiltransToken token;
	vector<wstring> tokens = wsplit(bt, L'/');
	
	token.sourceToken = parseTaggerToken(tokens[0]);

	for (int i = 1; i < tokens.size(); i++) {
		token.targetTokens.push_back(parseTaggerToken(tokens[i]));
	}
	return token;

}

void Multitrans::printBiltransSentence(int n, vector<BiltransToken> s) {
	wcout << n << "\t";
	for(int i = 0; i < s.size(); i++) {
		wcout << s[i].toString(true);
		if (i != s.size() - 1) {
			wcout << L" ";
		}
	}
	wcout << endl;
}

bool Multitrans::isPosAmbig(BiltransToken bt) {

	bool isPos;
	if (bt.sourceToken.tags.size() > 0) {
		isPos = 
		bt.sourceToken.tags[0] == L"n" ||
		bt.sourceToken.tags[0] == L"vblex" ||
		bt.sourceToken.tags[0] == L"adj";
	} else {
		isPos = false;
	}
	
	return isPos && bt.targetTokens.size() > 1;

}

BiltransToken Multitrans::getTrimmedToken(BiltransToken trimmed) {
	if(trimmed.sourceToken.lemma[0] == L'*') {
		return trimmed;
	}
	wstring str = bilingual.biltrans(
								trimmed.sourceToken.toString(false), false);
	if (str == L"") {
		str = L"@" + trimmed.toString(false);
	}
	BiltransToken newToken = parseBiltransToken(
								trimmed.sourceToken.toString(false) + L"/" + str);
	vector<wstring> newTags;
	


	for(int i = 0; i < trimmed.sourceToken.tags.size(); i++) {
		wstring tag = trimmed.sourceToken.tags[i];
		if (find(trimmed.targetTokens[0].tags, tag) ==
			find(newToken.targetTokens[0].tags, tag)) {
			newTags.push_back(tag);
		}
	}

	trimmed.sourceToken.tags = newTags;
	trimmed.targetTokens = trimmed.targetTokens;

	return trimmed;
	
}


void Multitrans::biltransToMultitrans(int sn, int &tn, int idx, 
	vector<BiltransToken> s, wstring buffer) 
{

	if (idx == s.size() ) {
		wcout << L".[][" <<  sn << L" " << tn << L"].[]\t" << buffer << endl;
		tn += 1;
		return;
	}
	int n = s[idx].targetTokens.size();
	wstring base;
	if( ! this->trimmed ) {
		base = L"^" + s[idx].sourceToken.toString(false) + L"/";
	} else {
		base = getTrimmedToken(s[idx]).sourceToken.toString(false) + L"/";
	}
	for(int i = 0; i < n; i++) {
		wstring token = base + s[idx].targetTokens[i].toString(false) + L"$";
		if(idx != s.size() - 1) {
			token += L" ";
		}
		biltransToMultitrans(sn, tn, idx+1, s, buffer + token);	
	}
}

void Multitrans::processSentence(vector<TaggerToken> sentence) {

	vector<BiltransToken> outputSentence;
	bool isAmbig = false;
	bool hasAmbigPos = false;
	int numberOfUnknown = 0;
	int fertility = 1;
	for(int i = 0; i < sentence.size(); i++) {
		wstring token = sentence[i].toString(false); 
		
		wstring target;
		if(this->trimmed){
			target = bilingual.biltransWithoutQueue(token, false);
		} else {
			target = bilingual.biltrans(token, false);
		}

		if (target == L"") {
			target = L"@" + token;
		}
		BiltransToken bt = parseBiltransToken(token + L'/' + target);

		if (bt.targetTokens.size() > 1) {
			isAmbig = true;
		}
		if (isPosAmbig(bt)) {
			hasAmbigPos = true;
		}
		if(token[0] == L'*') {
			numberOfUnknown ++;
		}
		fertility *= bt.targetTokens.size();
		outputSentence.push_back(bt);
		

	}
	double coverage = (100.0 - numberOfUnknown * 100.0 / sentence.size());

	if(fertility >= 2 && fertility <= 10000 && coverage >= 90.0) {
		wstring outBuffer = L"";
		int tn = 0;
		if(mode == "-b") {
			printBiltransSentence(this->sn, outputSentence);
		} else if (mode == "-m") {
			biltransToMultitrans(this->sn, tn, 0, outputSentence, outBuffer);

		}
	}
	this->sn ++;
	
}
