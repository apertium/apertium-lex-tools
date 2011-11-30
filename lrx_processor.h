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

#ifndef __LRX_PROCESSOR_H__
#define __LRX_PROCESSOR_H__

#include <cwchar>
#include <cstdio>
#include <libgen.h>
#include <cerrno>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <set>

#include <libxml/xmlreader.h>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/exception.h>
#include <lttoolbox/pool.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/state.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/trans_exe.h>

using namespace std;

typedef struct LSRuleExe
{
  int id;                       // id (e.g. line number) of the rule
  int len;                      // length of the pattern (in LUs)
  double weight;                // an arbitrary rule weight

} LSRuleExe;

typedef struct SItem
{
  wstring sl;
  vector<wstring> tl;
  wstring blank; // Superblank to the left
} SItem;

class LRXProcessor
{
private:
  Alphabet alphabet; // Alphabet of rule transducer
  TransExe transducer; // The rule transducer  
//  map<int, MatchExe> patterns; // Map of alphabet symbols to regex transducers 
  map<int, Transducer> patterns; // Map of alphabet symbols to regex transducers 
  map<int, LSRuleExe> rules; // Map of rule ids to rule info (weight, etc.)
  set<Node *> anfinals; // Final states 
  set<wchar_t> escaped_chars; // 'Special' characters to escape
  Pool<vector<int> > *pool;
  State *initial_state;
  State *current_state;
 
  bool traceMode; // Rule tracing ? 
  bool outOfWord; // Are we in a word ?
  int pos; // Current sentence position

  vector<int> pathsToRules(wstring const path);
  void readWord(SItem &w, FILE *input, FILE *output);
  void applyRules(map<int, SItem> &sentence, FILE *output);
  map< pair<int, int>, int > bestPath(map< pair<int, int>, vector<int> > &rule_spans, unsigned int len);

  wstring itow(int i);
  int wtoi(wstring w);

public:
  static wstring const LRX_PROCESSOR_S_BOUNDARY;

  LRXProcessor();
  ~LRXProcessor();

  void setTraceMode(bool mode);

  void init();
  void load(FILE *input);
  void process(FILE *input, FILE *output);

};

#endif /* __LRX_PROCESSOR_H__ */

