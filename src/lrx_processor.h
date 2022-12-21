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

#include <cstdio>
#include <libgen.h>
#include <set>
#include <cstdint>

#include <libxml/xmlreader.h>

#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/input_file.h>

using namespace std;

class LRXProcessor
{
private:

  Alphabet alphabet;
  TransExe transducer;
  map<UString, TransExe> recognisers;
  map<UString, double> weights;

  map<Node *, double> anfinals;
  set<UChar32> escaped_chars;
  State initial_state;

  bool traceMode = false;
  bool debugMode = false;
  bool nullFlush = false;

  int32_t any_char;
  int32_t any_upper;
  int32_t any_lower;
  int32_t any_tag;
  int32_t word_boundary;
  int32_t null_boundary;

  unsigned int pos = 0;
  unsigned long lineno = 1; // Used for rule tracing

  UString itow(int i);
  bool recognisePattern(const UString& lu, const UString& op);
  void read_seg(InputFile& input, UString& seg);
  void make_anys(int32_t sym, std::set<int32_t>& alts);

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
  static UString const LRX_PROCESSOR_TAG_ANY_CHAR;
  static UString const LRX_PROCESSOR_TAG_ANY_TAG;
  static UString const LRX_PROCESSOR_TAG_ANY_UPPER;
  static UString const LRX_PROCESSOR_TAG_ANY_LOWER;
  static UString const LRX_PROCESSOR_TAG_WORD_BOUNDARY;
  static UString const LRX_PROCESSOR_TAG_NULL_BOUNDARY;

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
