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

int findTag(vector<wstring> tags, wstring tag) {
	for (int i = 0; i < tags.size(); i++) {
		if (tags[i] == tag)
			return i;
	}
	return -1;

}

FSTProcessor loadBilingual(char **argv) {
	FSTProcessor bilingual;

	std::string resources_path(argv[1]);
	std::string direction(argv[2]);

	FILE *f_bin = fopen((resources_path + direction + ".autobil.bin").c_str(), "r");
	bilingual.load(f_bin);
	fclose(f_bin);
	bilingual.initBiltrans();
	return bilingual;
}

vector<wstring> parseTags(wstring token) {
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

vector<wstring> wsplit(wstring wstr, wchar_t delim) {
	vector<wstring> tokens;
	wstring buffer;

	for(int i = 0; i < wstr.size(); i++) {
		buffer += wstr[i];
		if(wstr[i] == delim) {
			tokens.push_back(buffer);
			buffer = L"";
		}
	}
	if(buffer != L"") {
		tokens.push_back(buffer);
	}
	return tokens;

}

wstring getLemma(wstring token) {
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

void processTaggerOutput(FSTProcessor *bilingual) {

	wstring buffer;

	bool escaped = false;
	int state = 0; // outside
	wchar_t c;
	bilingual->setBiltransSurfaceForms(true);
	while((wcin.get(c)) != NULL) {
		
		if (state == 0) {
			if (c == '^' && !escaped) {
				state = 1; // inside
				buffer += c;
			} else if (c == '\\' && !escaped) {
				wcout << c;
				escaped = true;
			} else {
				wcout << c;
				escaped = false;
			}
		} else if (state == 1) {
			if(c == L'$' && !escaped) {

				vector<wstring> sourceTags = parseTags(buffer);
				wstring target = bilingual->biltrans(buffer + L"$", true);
				vector<wstring> targetTags = parseTags(target);
				wstring targetTrimmed = bilingual->biltransWithoutQueue(buffer + L"$", true);
				vector<wstring> trimmedTags = parseTags(targetTrimmed);
				vector<wstring> newTags;

				for (int i = 0; i < sourceTags.size(); i++) {
					wstring sourceTag = sourceTags[i];
					int idx_1 = findTag(targetTags, sourceTag);
					int idx_2 = findTag(trimmedTags, sourceTag);
					if (idx_1 == idx_2){
						newTags.push_back(sourceTag);
					}
				}
				wcout << getLemma(buffer);
				for(int i = 0; i < newTags.size(); i++) {
					wcout << '<' << newTags[i] << '>';
				}
				targetTrimmed[0] = '/';
				if(targetTrimmed == L"/") {
					buffer[0] = L'@';
					wcout << L"/" + buffer + L"$";
				} else {
					vector<wstring> tokens = wsplit(targetTrimmed, '/');
					for(int i = 0; i < tokens.size(); i++) {
						wcout << tokens[i];
					}
				}

				buffer = L"";
				state = 0;
				escaped = false;


			} else if (c == '\\' && !escaped) {
				escaped = true;
			} else {
				buffer += c;
				escaped = false;
			}
		}
	}
}

int main(int argc, char **argv)
{
	if(argc != 3) { 
		wcout << L"Usage: " << argv[0] << "<language pair resources> <direction>"<< endl;
		exit(-1);
	}

    LtLocale::tryToSetLocale();
	FSTProcessor bilingual = loadBilingual(argv);
	processTaggerOutput(&bilingual);

	return 0;
}



