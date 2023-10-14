#include <lttoolbox/fst_processor.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/string_utils.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <lttoolbox/i18n.h>

int find(std::vector<UString> xs, UString x) {
	for (size_t i = 0; i < xs.size(); i++) {
		if (xs[i] == x)
			return i;
	}
	return -1;
}

FSTProcessor loadBilingual(char *path) {
	FSTProcessor bilingual;

	auto f_bin = fopen(path, "rb");
	bilingual.load(f_bin);
	fclose(f_bin);
	bilingual.initBiltrans();
	return bilingual;
}

std::vector<UString> parseTags(UString token) {
	bool in_tag = false;
	std::vector<UString> tags;
	UString buffer;
	for (size_t i = 0; i < token.size(); i++) {
		UChar c = token[i];
		if (!in_tag) {
			if (c == '<') {
				in_tag = true;
			}
		}
		else {
			if (c == '>') {
				tags.push_back(buffer);
				buffer.clear();
				in_tag = false;
			} else {
				buffer += c;
			}
		}
	}
	return tags;
}

UString getLemma(UString token) {
	UString buffer;
	for (size_t i = 0; i < token.size(); i++) {
		if (token[i] != '<') {
			buffer += token[i];
		} else {
			break;
		}
	}
	return buffer;
}

void processTaggerOutput(FSTProcessor *bilingual) {
	UString buffer;
	UChar32 c;
	bilingual->setBiltransSurfaceForms(true);
	InputFile in;
  while (!in.eof()) {
    std::cout << in.readBlank(true);
    if (in.eof()) {
      break;
    }
    c = in.get();
    if (c == '^') {
      buffer = in.readBlock('^', '$');
      auto sourceTags = parseTags(buffer);
      auto target = bilingual->biltrans(buffer, true);
      auto targetTags = parseTags(target);
      auto targetTrimmed = bilingual->biltransWithoutQueue(buffer, true);
      auto trimmedTags = parseTags(targetTrimmed);
      std::vector<UString> newTags;

      for (size_t i = 0; i < sourceTags.size(); i++) {
        UString sourceTag = sourceTags[i];
        auto idx_1 = find(targetTags, sourceTag);
        auto idx_2 = find(trimmedTags, sourceTag);
        if (idx_1 == idx_2){
          newTags.push_back(sourceTag);
        }
      }
      std::cout << getLemma(buffer);
      for (size_t i = 0; i < newTags.size(); i++) {
        std::cout << '<' << newTags[i] << '>';
      }
      targetTrimmed[0] = '/';
      if (targetTrimmed.size() == 1) {
        buffer[0] = '@';
        std::cout << '/' << buffer;
      } else {
        auto tokens = StringUtils::split(targetTrimmed, "/"_u);
        for (auto& token : tokens) {
          std::cout << '/' << token;
        }
      }

      buffer.clear();
    }
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << I18n(ALX_I18N_DATA, "alx").format("process_tagger_output_desc", {"program"}, {argv[0]})
		          << std::endl;
		exit(-1);
	}

  LtLocale::tryToSetLocale();
	FSTProcessor bilingual = loadBilingual(argv[1]);
	processTaggerOutput(&bilingual);
}
