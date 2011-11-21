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

class LRXProcessor
{
private:
  Alphabet alphabet;
  TransExe transducer;
  map<int, MatchExe> patterns;
  map<int, LSRuleExe> rules;
  set<Node *> anfinals;
  set<wchar_t> escaped_chars;
  Pool<vector<int> > *pool;
  State *current_state;
  State *initial_state;
 
  bool traceMode;
  

  void streamError();
  wstring itow(int i);
  int wtoi(wstring w);

public:
  LRXProcessor();
  ~LRXProcessor();

  void setTraceMode(bool mode);

  void load(FILE *input);
  void process(FILE *input, FILE *output);

};

#endif /* __LRX_PROCESSOR_H__ */

