#include "multi_translator.h"

MultiTranslator::MultiTranslator(string path, string mode, bool trimmed, bool filter, bool number_lines) {
	this->trimmed = trimmed;
	this->filter = filter;
	this->number_lines = number_lines;

	this->path = path;
	this->mode = mode;

	FILE *f_bin = fopen(path.c_str(), "r");
	bilingual.load(f_bin);
	fclose(f_bin);
	bilingual.initBiltrans();
	bilingual.setDictionaryCaseMode(true);
}
MultiTranslator::~MultiTranslator() {}

int MultiTranslator::calculateFertility(vector<BiltransToken> sent) {
	unsigned int fertility = 1;
	for (unsigned int i = 0; i < sent.size(); i++) {
		fertility *= sent[i].targetTokens.size();
	}
	return fertility;
}


BiltransToken MultiTranslator::parseBiltransToken(wstring bt) {

	BiltransToken token;
	vector<wstring> tokens = wsplit(bt, L'/');
	
	token.sourceToken = parseTaggerToken(tokens[0]);

	for (unsigned int i = 1; i < tokens.size(); i++) {
		token.targetTokens.push_back(parseTaggerToken(tokens[i]));
	}
	return token;

}

bool MultiTranslator::isPosAmbig(BiltransToken bt) {

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

BiltransToken MultiTranslator::getFullToken(wstring source) {

	BiltransToken token;	
	if (source[0] == L'*') {
		token.sourceToken.lemma = source;
		TaggerToken tmp;
		tmp.lemma = source;
		token.targetTokens.push_back(tmp);
		return token;
	}

	wstring target = bilingual.biltrans(source, false);
	if (target == L"") {
		target = L"@" + source;
	} 
	token = parseBiltransToken(source + L"/" + target);
	return token;
	
}

BiltransToken MultiTranslator::getTrimmedToken(wstring source) {

	BiltransToken ttoken;
	BiltransToken ftoken;

	if (source[0] == L'*') {
		ttoken.sourceToken.lemma = source;
		TaggerToken tmp;
		tmp.lemma = source;
		ttoken.targetTokens.push_back(tmp);
		return ttoken;
	}

	wstring fstr = bilingual.biltrans(source, false);
	wstring tstr = bilingual.biltransWithoutQueue(source, false);

	if (fstr == L"") {
		fstr = L"@" + source;
	} 
	if (tstr == L"") {
		tstr = L"@" + source;
	}

	ttoken = parseBiltransToken(source + L"/" + tstr);
	ftoken = parseBiltransToken(source + L"/" + fstr);


	if(this->trimmed) {
		for(unsigned int i = 0; i < ftoken.targetTokens.size(); i++ ) {
			if(ttoken.targetTokens[i].tags.size() < 
			   ftoken.targetTokens[i].tags.size()) {
				ttoken.targetTokens[i].tags.push_back(L"*");
			}
		}
	}

	vector<wstring> newTags;
	//bool sourceTrimmed = false;
	for(unsigned int i = 0; i < ttoken.sourceToken.tags.size(); i++) {
		wstring tag = ttoken.sourceToken.tags[i];
		if (find(ttoken.targetTokens[0].tags, tag) ==
			find(ftoken.targetTokens[0].tags, tag)) {
			newTags.push_back(tag);
		}
	}
	if(ttoken.sourceToken.tags.size() > newTags.size()) {
		newTags.push_back(L"*");
	}
	ttoken.sourceToken.tags = newTags;

	return ttoken;
	
}

void MultiTranslator::biltransToMultiTranslator(int sn, int &tn, unsigned int idx, 
	vector<BiltransToken> s, wstring buffer) 
{

	if (idx == s.size() ) {
		wcout << L".[][" <<  sn << L" " << tn << L"].[]\t" << buffer << endl;
		tn += 1;
		return;
	}
	int n = s[idx].targetTokens.size();
	wstring base;
	base = s[idx].sourceToken.toString(false) + L"/";
	for(int i = 0; i < n; i++) {
		wstring token = L"^" + base + s[idx].targetTokens[i].toString(false) + L"$";
		if(idx != s.size() - 1) {
			token += L" ";
		}
		biltransToMultiTranslator(sn, tn, idx+1, s, buffer + token);	
	}
}
void MultiTranslator::printBiltransSentence(int n, vector<BiltransToken> s) {
	if (number_lines) {
		wcout << n << "\t";
	}
	for(unsigned int i = 0; i < s.size(); i++) {
		wcout << s[i].toString(true);
		if (i != s.size() - 1) {
			wcout << L" ";
		}
	}
	wcout << endl;
}

void MultiTranslator::printTaggerOutput(int n, vector<BiltransToken> sentence) {
	if (number_lines) {
		wcout << n << "\t";
	}

	for(unsigned int i = 0; i < sentence.size(); i++) {
		wcout << sentence[i].sourceToken.toString(true);
		if (i != sentence.size() -1) {
			wcout << L" ";
		}
	}
	wcout << endl;
}

void MultiTranslator::processSentence(vector<TaggerToken> sentence) {

	vector<BiltransToken> outputSentence;
	bool isAmbig = false;
	bool hasAmbigPos = false;
	int numberOfUnknown = 0;
	int fertility = 1;
	for(unsigned int i = 0; i < sentence.size(); i++) {
		wstring token = sentence[i].toString(false); 
		wstring target;
	
		BiltransToken bt;
		if(this->trimmed){
			bt = getTrimmedToken(token);			
		} else {
			bt = getFullToken(token);
		}

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
	double coverage = (1.0 * sentence.size() - 2* numberOfUnknown) / sentence.size() * 100;

	bool flag = (this->filter == false) || (this->filter && fertility >= 2 && 
				fertility <= 10000 && coverage >= 90.0 && hasAmbigPos);


	if (flag) {
		if (mode == "-p") {
			printTaggerOutput(this->sn, outputSentence);
		} else if(mode == "-b") {
			printBiltransSentence(this->sn, outputSentence);
		} else if (mode == "-m") {
			wstring outBuffer = L"";
			int tn = 0;
			biltransToMultiTranslator(this->sn, tn, 0, outputSentence, outBuffer);
		} 
	}
	this->sn ++;
	
}
