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

#include <string>
#include <cstdint>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/alphabet.h>
#include <unicode/ustdio.h>

using namespace std;

class LRXCompiler
{
private:
  Alphabet alphabet;
  Transducer transducer;

  map<UString, Transducer> recognisers; // keyed on pattern
  map<int32_t, double> weights; // keyed on rule id

  map<UString, Transducer> sequences;

  map<UString, xmlNode*> macros;
  vector<UString> macro_string_vars;
  vector<xmlNode*> macro_node_vars;
  xmlNode* currentMacro = nullptr;

  int32_t initialState;
  int32_t currentState;

  int32_t currentRuleId = 0;

  int32_t any_tag = 0;
  int32_t any_char = 0;
  int32_t any_upper = 0;
  int32_t any_lower = 0;
  int32_t word_boundary = 0;
  int32_t null_boundary = 0;
  int32_t select_sym = 0;
  int32_t remove_sym = 0;
  int32_t skip_sym = 0;

  bool globIsStar = false;

  bool debugMode = false;
  bool outputGraph = false;
  UFILE* debug_output;
  void debug(const char* fmt, ...);

  UString attr(xmlNode* node, const UString& attr, const UString& fallback);
  UString attr(xmlNode* node, const UString& attr);

  void skipBlanks(UString &name);
  void procNode(xmlNode* node);
  void procList(xmlNode* node);
  void procListMatch(xmlNode* node);
  void procRule(xmlNode* node);
  void procDefSeq(xmlNode* node);
  void procOr(xmlNode* node);
  int compileSpecifier(xmlNode* node, Transducer* t, int state, UString* key);
  void compileSequence(xmlNode* node);
  void procMatch(xmlNode* node);
  void procSelectRemove(xmlNode* node);
  void procRepeat(xmlNode* node);
  void procSeq(xmlNode* node);
  void procMacro(xmlNode* node);

public:
  static UString const LRX_COMPILER_LRX_ELEM;
  static UString const LRX_COMPILER_DEFMACROS_ELEM;
  static UString const LRX_COMPILER_DEFMACRO_ELEM;
  static UString const LRX_COMPILER_DEFSEQS_ELEM;
  static UString const LRX_COMPILER_DEFSEQ_ELEM;
  static UString const LRX_COMPILER_RULES_ELEM;
  static UString const LRX_COMPILER_RULE_ELEM;
  static UString const LRX_COMPILER_MACRO_ELEM;
  static UString const LRX_COMPILER_MATCH_ELEM;
  static UString const LRX_COMPILER_SELECT_ELEM;
  static UString const LRX_COMPILER_REMOVE_ELEM;
  static UString const LRX_COMPILER_OR_ELEM;
  static UString const LRX_COMPILER_REPEAT_ELEM;
  static UString const LRX_COMPILER_SEQ_ELEM;
  static UString const LRX_COMPILER_BEGIN_ELEM;
  static UString const LRX_COMPILER_PARAM_ELEM;
  static UString const LRX_COMPILER_WITH_PARAM_ELEM;

  static UString const LRX_COMPILER_SURFACE_ATTR;
  static UString const LRX_COMPILER_SUFFIX_ATTR;
  static UString const LRX_COMPILER_LEMMA_ATTR;
  static UString const LRX_COMPILER_CONTAINS_ATTR;
  static UString const LRX_COMPILER_CASE_ATTR;
  static UString const LRX_COMPILER_TAGS_ATTR;
  static UString const LRX_COMPILER_COMMENT_ATTR;
  static UString const LRX_COMPILER_NAME_ATTR;
  static UString const LRX_COMPILER_NODES_ATTR;
  static UString const LRX_COMPILER_NPAR_ATTR;
  static UString const LRX_COMPILER_VALUE_ATTR;
  static UString const LRX_COMPILER_WEIGHT_ATTR;
  static UString const LRX_COMPILER_FROM_ATTR;
  static UString const LRX_COMPILER_UPTO_ATTR;
  static UString const LRX_COMPILER_GLOB_ATTR;
  static UString const LRX_COMPILER_GLOB_PLUS_VAL;
  static UString const LRX_COMPILER_GLOB_STAR_VAL;

  static UString const LRX_COMPILER_TYPE_MATCH;
  static UString const LRX_COMPILER_TYPE_SELECT;
  static UString const LRX_COMPILER_TYPE_REMOVE;
  static UString const LRX_COMPILER_TYPE_SKIP;

  static UString const LRX_COMPILER_SYM_SELECT;
  static UString const LRX_COMPILER_SYM_REMOVE;
  static UString const LRX_COMPILER_SYM_SKIP;

  static double  const LRX_COMPILER_DEFAULT_WEIGHT;


  LRXCompiler();

  ~LRXCompiler();

  void parse(string const &fitxer);

  void write(FILE *fd);

  void setOutputGraph(bool o);
  void setDebugMode(bool o);

};

#endif /* __LRX_COMPILER_H__ */
