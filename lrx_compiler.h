#ifndef __LEX_RULE_COMPILER_H__
#define __LEX_RULE_COMPILER_H__

#include <cwchar>
#include <cstdio>
#include <libgen.h>
#include <cerrno>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <list>
#include <set>

#include <libxml/xmlreader.h>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/pool.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>


using namespace std;

class LRXCompiler
{
private:
  xmlTextReaderPtr reader;
  Alphabet alphabet;
  Transducer transducer;  
  map<int, Transducer> patterns;
  int current_line;
  int current_rule;
  
  void procNode();


public:
  static wstring const LRX_COMPILER_RULES_ELEM;
  static wstring const LRX_COMPILER_RULE_ELEM;
  static wstring const LRX_COMPILER_SKIP_ELEM;
  static wstring const LRX_COMPILER_SELECT_ELEM;
  static wstring const LRX_COMPILER_REMOVE_ELEM;
  static wstring const LRX_COMPILER_ACCEPTION_ELEM;
  static wstring const LRX_COMPILER_OR_ELEM;

  static wstring const LRX_COMPILER_LEMMA_ATTR;
  static wstring const LRX_COMPILER_TAGS_ATTR;
  static wstring const LRX_COMPILER_C_ATTR;

  LRXCompiler();

  ~LRXCompiler();

  void parse(string const &fitxer);

  void write(FILE *fd);
};

#endif
