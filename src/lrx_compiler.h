/*
 * Copyright (C) 2011--2012 Universitat d'Alacant
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __LRX_COMPILER_H__
#define __LRX_COMPILER_H__

#include <cwchar>
#include <cstdio>
#include <libgen.h>
#include <cerrno>
#include <string>
#include <iostream>
#include <limits>
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
#include <lttoolbox/compression.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/my_stdio.h>

#include <unicode/ustdio.h>

using namespace std;

class LRXCompiler
{
private:
  xmlTextReaderPtr reader;
  Alphabet alphabet;
  Transducer transducer;

  map<UString, Transducer> recognisers; // keyed on pattern
  map<int, double> weights; // keyed on rule id

  map<UString, Transducer> sequences;

  int initialState;
  int lastState;
  int currentState;
  bool canSelect; // disallow <select>, <remove> inside <def-seq>, <repeat>

  int currentRuleId;

  bool debugMode;
  bool outputGraph;
  UFILE* debug_output;
  void debug(const char* fmt, ...);
  bool allBlanks();

  void skipBlanks(UString &name);
  void procNode();
  void procList();
  void procListMatch();
  void procRule();
  void procDefSeq();
  void procOr();
  void procMatch();
  void procSelect();
  void procRemove();
  void procRepeat();
  void procSeq();

  /* If attrib does not exist (or other error), returns an empty string: */
  UString attrib(UString const &name);

  /* If attrib does not exist (or other error), returns fallback: */
  UString attrib(UString const &name, const UString fallback);

  UString itow(int i);
  int wtoi(UString);
  double wtod(UString);

public:
  static UString const LRX_COMPILER_LRX_ELEM;
  static UString const LRX_COMPILER_DEFSEQS_ELEM;
  static UString const LRX_COMPILER_DEFSEQ_ELEM;
  static UString const LRX_COMPILER_RULES_ELEM;
  static UString const LRX_COMPILER_RULE_ELEM;
  static UString const LRX_COMPILER_MATCH_ELEM;
  static UString const LRX_COMPILER_SELECT_ELEM;
  static UString const LRX_COMPILER_REMOVE_ELEM;
  static UString const LRX_COMPILER_OR_ELEM;
  static UString const LRX_COMPILER_REPEAT_ELEM;
  static UString const LRX_COMPILER_SEQ_ELEM;

  static UString const LRX_COMPILER_SURFACE_ATTR;
  static UString const LRX_COMPILER_SUFFIX_ATTR;
  static UString const LRX_COMPILER_LEMMA_ATTR;
  static UString const LRX_COMPILER_CONTAINS_ATTR;
  static UString const LRX_COMPILER_CASE_ATTR;
  static UString const LRX_COMPILER_TAGS_ATTR;
  static UString const LRX_COMPILER_COMMENT_ATTR;
  static UString const LRX_COMPILER_NAME_ATTR;
  static UString const LRX_COMPILER_WEIGHT_ATTR;
  static UString const LRX_COMPILER_FROM_ATTR;
  static UString const LRX_COMPILER_UPTO_ATTR;

  static UString const LRX_COMPILER_TYPE_SELECT;
  static UString const LRX_COMPILER_TYPE_REMOVE;
  static UString const LRX_COMPILER_TYPE_SKIP;

  static double  const LRX_COMPILER_DEFAULT_WEIGHT;


  LRXCompiler();

  ~LRXCompiler();

  void parse(string const &fitxer);

  void write(FILE *fd);

  void setOutputGraph(bool o);
  void setDebugMode(bool o);

};

#endif /* __LRX_COMPILER_H__ */

