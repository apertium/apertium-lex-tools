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
#include <cmath>
#include <sstream>
#include <limits>
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
#include <lttoolbox/compression.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/state.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/my_stdio.h>

using namespace std;
/*
class BiltransToken {
public:
	bool isEOF = false;
	wstring source;
	wstring blanks;
	vector<wstring> target;

	wstring toString(bool delim) {
		wstring out = source;
		for(int i = 0; i < target.size(); i++) {
			out += L'/' + target[i];
		}
		if (delim && (source.size() > 0 || target.size() > 0)) {
			out = blanks + L'^' + out + L'$';
		} else {
			out = blanks + out;
		}
		return out;	
	}
};

*/
class LRXProcessor
{
private:

  Alphabet alphabet;
  TransExe transducer;
  map<wstring, TransExe> recognisers;
  map<wstring, double> weights;

//  map<int, BiltransToken> bts;

  vector<State> alive_states;

  set<Node *> anfinals;
  set<wchar_t> escaped_chars;
  State *initial_state;

  bool traceMode;
  bool debugMode;
  bool nullFlush;
  bool outOfWord;

  unsigned int pos; 
  unsigned long lineno;

  wstring itow(int i);
  bool recognisePattern(const wstring lu, const wstring op);
  wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);

//  BiltransToken readBiltransToken(FILE *input = stdin);

  void makeTransition(int);
  void filterFinals();
  void evaluateRules();

  void processFlush(FILE *output,
		    map<int, wstring > &sl,
		    map<int, vector<wstring> > &tl,
		    map<int, wstring > &blanks,
		    map<int, pair<double, vector<State> > > &covers,
		    pair<double, vector<State> > &empty_seq,
		    map<pair<int, int>, vector<State> > &spans,
		    int last_final);

public:
  static wstring const LRX_PROCESSOR_TAG_SELECT;
  static wstring const LRX_PROCESSOR_TAG_REMOVE;
  static wstring const LRX_PROCESSOR_TAG_SKIP;

  LRXProcessor();
  ~LRXProcessor();

  void setTraceMode(bool mode);
  void setDebugMode(bool mode);
  void setNullFlush(bool mode);

  void init();
  void load(FILE *input);
  void process(FILE *input, FILE *output);
  void processME(FILE *input, FILE *output);

};

#endif /* __LRX_PROCESSOR_H__ */

