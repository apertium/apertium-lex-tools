/*
 * Copyright (C) 2011 Universitat d'Alacant 
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __LRX_COMPILER_H__
#define __LRX_COMPILER_H__

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

typedef struct LSRule
{
  int line;                     // line number
  int id;			// id of the rule
  int len;			// length of the pattern (in LUs)
  double weight;		// an arbitrary rule weight
  int ops;                      // number of (non-skip) operations in the current rule
  vector<wstring> tl_patterns; 	// patterns

  map<int, vector<wstring> > sl_context; // position,pattern
  map<int, vector<wstring> >  tl_context;  // 

} LSRule;

typedef struct LSRuleRecord
{
  int id;			// id (e.g. line number) of the rule
  int len;			// length of the pattern (in LUs)
  int ops;			// number of (non-skip) operations
  double weight;		// an arbitrary rule weight

} LSRuleRecord;

class LRXCompiler
{
private:
  xmlTextReaderPtr reader;
  Alphabet alphabet;
  Transducer transducer;  
  map<int, Transducer> patterns;
  map<int, LSRule> rules;

  int current_line;
  int current_rule_id;
  int current_rule_len;
  int current_context_pos;
  wstring current_pattern;

  bool outputGraph;
  
  bool allBlanks();
  void skipBlanks(wstring &name);
  void procNode();
  void procRule();
  void procOr();
  void procMatch();
  void procSelect();
  void procRemove();
  wstring attrib(wstring const &name);
  wstring attribsToPattern(wstring lemma, wstring tags);
  wstring operationToPattern(wstring op);

  wstring itow(int i);
  int wtoi(wstring);


public:
  static wstring const LRX_COMPILER_RULES_ELEM;
  static wstring const LRX_COMPILER_RULE_ELEM;
  static wstring const LRX_COMPILER_MATCH_ELEM;
  static wstring const LRX_COMPILER_SELECT_ELEM;
  static wstring const LRX_COMPILER_REMOVE_ELEM;
  static wstring const LRX_COMPILER_OR_ELEM;

  static wstring const LRX_COMPILER_LEMMA_ATTR;
  static wstring const LRX_COMPILER_TAGS_ATTR;
  static wstring const LRX_COMPILER_C_ATTR;

  static wstring const LRX_COMPILER_TYPE_SELECT;
  static wstring const LRX_COMPILER_TYPE_REMOVE;

  static wstring const LRX_COMPILER_ASTERISK;

  static double  const LRX_COMPILER_DEFAULT_WEIGHT;

  LRXCompiler();

  ~LRXCompiler();

  void parse(string const &fitxer);

  void write(FILE *fd);

  void setOutputGraph(bool o);
};

#endif /* __LRX_COMPILER_H__ */
