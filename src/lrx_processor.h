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

#ifndef __LRX_PROCESSOR_H__
#define __LRX_PROCESSOR_H__

#include <cwchar>
#include <cstdio>
#include <libgen.h>
#include <cerrno>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <set>

#include <libxml/xmlreader.h>

#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/exception.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/state.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/my_stdio.h>
#include <lttoolbox/input_file.h>

using namespace std;

class LRXProcessor
{
private:

  Alphabet alphabet;
  TransExe transducer;
  map<UString, TransExe> recognisers;
  map<UString, double> weights;

  vector<State> alive_states;

  map<Node *, double> anfinals;
  set<UChar32> escaped_chars;
  State *initial_state;

  bool traceMode;
  bool debugMode;
  bool nullFlush;
  bool outOfWord;

  unsigned int pos;
  unsigned long lineno;

  UString itow(int i);
  bool recognisePattern(const UString lu, const UString op);
  UString readFullBlock(InputFile& input, UChar32 const delim1, UChar32 const delim2);

  void makeTransition(int);
  void filterFinals();
  void evaluateRules();

  enum OpType { Select, Remove };

  void processFlush(UFILE *output,
                      map<int, UString > &sl,
                      map<int, vector<UString> > &tl,
                      map<int, UString > &blanks,
                      map<int, map<UString, double> > &scores,
                      map<int, map<UString, OpType> > &operations);

public:
  static UString const LRX_PROCESSOR_TAG_SELECT;
  static UString const LRX_PROCESSOR_TAG_REMOVE;
  static UString const LRX_PROCESSOR_TAG_SKIP;

  LRXProcessor();
  ~LRXProcessor();

  void setTraceMode(bool mode);
  void setDebugMode(bool mode);
  void setNullFlush(bool mode);

  void init();
  void load(FILE *input);
  void process(InputFile& input, UFILE *output);
};

#endif /* __LRX_PROCESSOR_H__ */
