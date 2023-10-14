#include "multi_translator.h"
#include <iostream>
#include <lttoolbox/i18n.h>

MultiTranslator::MultiTranslator(string path, string mode, bool trimmed, bool filter, bool number_lines) {
	this->trimmed = trimmed;
	this->filter = filter;
	this->number_lines = number_lines;

	this->path = path;
	this->mode = mode;

	FILE *f_bin = fopen(path.c_str(), "rb");
	if(!f_bin)
		I18n(ALX_I18N_DATA, "alx").error("ALX80000", {"file"}, {path.c_str()}, true);
	bilingual.load(f_bin);
	fclose(f_bin);
	bilingual.initBiltrans();
	bilingual.setDictionaryCaseMode(true);
}
MultiTranslator::~MultiTranslator() {}

int MultiTranslator::calculateFertility(vector<BiltransToken> sent) {
	unsigned int fertility = 1;
	for (auto& si : sent) {
		fertility *= si.targetTokens.size();
	}
	return fertility;
}


BiltransToken MultiTranslator::parseBiltransToken(UString bt) {

	BiltransToken token;
	vector<UString> tokens = wsplit(bt, '/');

	token.sourceToken = parseTaggerToken(tokens[0]);

	for (size_t i = 1; i < tokens.size(); i++){
		token.targetTokens.push_back(parseTaggerToken(tokens[i]));
	}
	return token;

}

bool MultiTranslator::isPosAmbig(BiltransToken bt) {

	bool isPos;
	if (bt.sourceToken.tags.size() > 0) {
		isPos =
		bt.sourceToken.tags[0] == "n"_u ||
		bt.sourceToken.tags[0] == "vblex"_u ||
		bt.sourceToken.tags[0] == "adj"_u;
	} else {
		isPos = false;
	}

	return isPos && bt.targetTokens.size() > 1;

}

BiltransToken MultiTranslator::getFullToken(UString source) {

	BiltransToken token;
	if (source[0] == '*') {
		token.sourceToken.lemma = source;
		TaggerToken tmp;
		tmp.lemma = source;
		token.targetTokens.push_back(tmp);
		return token;
	}

	UString target = bilingual.biltrans(source, false);
	if (target.empty()) {
      target += '@';
      target.append(source);
	}
	token = parseBiltransToken(source + "/"_u + target);
	return token;

}

BiltransToken MultiTranslator::getTrimmedToken(UString source)
{
	BiltransToken ttoken;
	BiltransToken ftoken;

	if (source[0] == '*') {
		ttoken.sourceToken.lemma = source;
		TaggerToken tmp;
		tmp.lemma = source;
		ttoken.targetTokens.push_back(tmp);
		return ttoken;
	}

        /*---------------------------------------------*/

        // This code is to attempt to avoid memory leak problems when calling
        // the bilingual.* methods in FSTProcessor. Unknown why we get the
        // leaks in the first place...

        UString fstr;
        UString tstr;

	if((f_cache.find(source) == f_cache.end()))
        {
	  f_cache[source] = bilingual.biltrans(source, false);
        }
	if((t_cache.find(source) == t_cache.end()))
        {
	  t_cache[source] = bilingual.biltransWithoutQueue(source, false);
        }

        fstr = f_cache[source];
        tstr = t_cache[source];

        /*---------------------------------------------*/

        if (fstr.empty()) {
          fstr += '@';
          fstr.append(source);
        }
        if (tstr.empty()) {
          tstr += '@';
          tstr.append(source);
        }

	ttoken = parseBiltransToken(source + "/"_u + tstr);
	ftoken = parseBiltransToken(source + "/"_u + fstr);


	if(this->trimmed) {
		for(size_t i = 0; i < ftoken.targetTokens.size(); ++i ) {
			if(ttoken.targetTokens[i].tags.size() <
			   ftoken.targetTokens[i].tags.size()) {
				ttoken.targetTokens[i].tags.push_back("*"_u);
			}
		}
	}

	vector<UString> newTags;
	//bool sourceTrimmed = false;
	for(size_t i = 0; i < ttoken.sourceToken.tags.size(); ++i) {
		UString tag = ttoken.sourceToken.tags[i];
		if ((int)i ==
			find(ftoken.targetTokens[0].tags, tag)) {
			newTags.push_back(tag);
		}
	}
	if(ttoken.sourceToken.tags.size() > newTags.size()) {
		newTags.push_back("*"_u);
	}
	ttoken.sourceToken.tags = newTags;

	return ttoken;
}

void MultiTranslator::biltransToMultiTranslator(int sn, int &tn, unsigned int idx,
	vector<BiltransToken> s, UString buffer)
{

	if (idx == s.size() ) {
		cout << ".[][" <<  sn << " " << tn << "].[]\t" << buffer << endl;
		tn += 1;
		return;
	}
	auto n = s[idx].targetTokens.size();
	UString base;
	base = s[idx].sourceToken.toString(false) + "/"_u;
	for(size_t i = 0; i < n; ++i) {
		UString token = "^"_u + base + s[idx].targetTokens[i].toString(false) + "$"_u;
		if(idx != s.size() - 1) {
			token += ' ';
		}
		biltransToMultiTranslator(sn, tn, idx+1, s, buffer + token);
	}
}
void MultiTranslator::printBiltransSentence(int n, vector<BiltransToken> s) {
	if (number_lines) {
		cout << n << "\t";
	}
	for(size_t i = 0; i < s.size(); ++i) {
		cout << s[i].toString(true);
		if (i != s.size() - 1) {
			cout << " ";
		}
	}
	cout << endl;
}

void MultiTranslator::printTaggerOutput(int n, vector<BiltransToken> sentence) {
	if (number_lines) {
		cout << n << "\t";
	}

	for(size_t i = 0; i < sentence.size(); ++i) {
		cout << sentence[i].sourceToken.toString(true);
		if (i != sentence.size() -1) {
			cout << " ";
		}
	}
	cout << endl;
}

void MultiTranslator::processSentence(vector<TaggerToken> sentence) {

	vector<BiltransToken> outputSentence;
	bool hasAmbigPos = false;
	int numberOfUnknown = 0;
	int fertility = 1;
	for(size_t i = 0; i < sentence.size(); ++i) {
		UString token = sentence[i].toString(false);
		UString target;

		BiltransToken bt;
		if(this->trimmed){
			bt = getTrimmedToken(token);
		} else {
			bt = getFullToken(token);
		}

		if (isPosAmbig(bt)) {
			hasAmbigPos = true;
		}
		if(token[0] == '*') {
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
			UString outBuffer;
			int tn = 0;
			biltransToMultiTranslator(this->sn, tn, 0, outputSentence, outBuffer);
		}
	}
	++this->sn;

}
