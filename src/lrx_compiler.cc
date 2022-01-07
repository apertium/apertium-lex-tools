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

#include <lrx_compiler.h>
#include <weight.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/compression.h>
#include <iostream>
#include <limits>

using namespace std;

UString const LRXCompiler::LRX_COMPILER_LRX_ELEM        = "lrx"_u;
UString const LRXCompiler::LRX_COMPILER_DEFSEQS_ELEM    = "def-seqs"_u;
UString const LRXCompiler::LRX_COMPILER_DEFSEQ_ELEM     = "def-seq"_u;
UString const LRXCompiler::LRX_COMPILER_RULES_ELEM      = "rules"_u;
UString const LRXCompiler::LRX_COMPILER_RULE_ELEM       = "rule"_u;
UString const LRXCompiler::LRX_COMPILER_MATCH_ELEM      = "match"_u;
UString const LRXCompiler::LRX_COMPILER_SELECT_ELEM     = "select"_u;
UString const LRXCompiler::LRX_COMPILER_REMOVE_ELEM     = "remove"_u;
UString const LRXCompiler::LRX_COMPILER_OR_ELEM         = "or"_u;
UString const LRXCompiler::LRX_COMPILER_REPEAT_ELEM     = "repeat"_u;
UString const LRXCompiler::LRX_COMPILER_SEQ_ELEM        = "seq"_u;

UString const LRXCompiler::LRX_COMPILER_LEMMA_ATTR      = "lemma"_u;
UString const LRXCompiler::LRX_COMPILER_SUFFIX_ATTR     = "suffix"_u;
UString const LRXCompiler::LRX_COMPILER_CONTAINS_ATTR   = "contains"_u;
UString const LRXCompiler::LRX_COMPILER_CASE_ATTR       = "case"_u;
UString const LRXCompiler::LRX_COMPILER_SURFACE_ATTR    = "surface"_u;
UString const LRXCompiler::LRX_COMPILER_TAGS_ATTR       = "tags"_u;
UString const LRXCompiler::LRX_COMPILER_WEIGHT_ATTR     = "weight"_u;
UString const LRXCompiler::LRX_COMPILER_COMMENT_ATTR    = "c"_u;
UString const LRXCompiler::LRX_COMPILER_NAME_ATTR       = "n"_u;
UString const LRXCompiler::LRX_COMPILER_FROM_ATTR       = "from"_u;
UString const LRXCompiler::LRX_COMPILER_UPTO_ATTR       = "upto"_u;

UString const LRXCompiler::LRX_COMPILER_TYPE_MATCH      = "match"_u;
UString const LRXCompiler::LRX_COMPILER_TYPE_SELECT     = "select"_u;
UString const LRXCompiler::LRX_COMPILER_TYPE_REMOVE     = "remove"_u;
UString const LRXCompiler::LRX_COMPILER_TYPE_SKIP       = "skip"_u;

double const  LRXCompiler::LRX_COMPILER_DEFAULT_WEIGHT  = 1.0;

void
LRXCompiler::debug(const char* fmt, ...)
{
  if (debugMode) {
    va_list argptr;
    va_start(argptr, fmt);
    u_vfprintf(debug_output, fmt, argptr);
    va_end(argptr);
  }
}

void
LRXCompiler::error(const char* fmt, ...)
{
  u_fprintf(debug_output, "Error (line %d): ",
            xmlTextReaderGetParserLineNumber(reader));
  va_list argptr;
  va_start(argptr, fmt);
  u_vfprintf(debug_output, fmt, argptr);
  va_end(argptr);
  u_fputc('\n', debug_output);
  exit(EXIT_FAILURE);
}

LRXCompiler::LRXCompiler()
{
  debug_output = u_finit(stderr, NULL, NULL);

  initialState = transducer.getInitial();
  currentState = initialState;
  lastState = initialState;

  alphabet.includeSymbol("<"_u+ LRX_COMPILER_TYPE_SELECT + ">"_u);
  alphabet.includeSymbol("<"_u+ LRX_COMPILER_TYPE_REMOVE + ">"_u);
  alphabet.includeSymbol("<"_u+ LRX_COMPILER_TYPE_SKIP + ">"_u);

  alphabet.includeSymbol("<ANY_TAG>"_u);
  alphabet.includeSymbol("<ANY_CHAR>"_u);
  alphabet.includeSymbol("<ANY_UPPER>"_u);
  alphabet.includeSymbol("<ANY_LOWER>"_u);
  alphabet.includeSymbol("<$>"_u);

  any_tag        = alphabet("<ANY_TAG>"_u);
  any_char       = alphabet("<ANY_CHAR>"_u);
  any_upper      = alphabet("<ANY_UPPER>"_u);
  any_lower      = alphabet("<ANY_LOWER>"_u);
  word_boundary  = alphabet(alphabet("<$>"_u), alphabet("<$>"_u));
}

LRXCompiler::~LRXCompiler()
{
}

void
LRXCompiler::setDebugMode(bool o)
{
  debugMode = o;
}


void
LRXCompiler::setOutputGraph(bool o)
{
  outputGraph = o;
}

void
LRXCompiler::skipBlanks(UString &name)
{
  while(name == "#text"_u || name == "#comment"_u)
  {
    if(name != "#comment"_u)
    {
      if(!allBlanks())
      {
        error("Invalid construction.");
      }
    }

    xmlTextReaderRead(reader);
    name = XMLParseUtil::readName(reader);
  }
}

UString
LRXCompiler::attrib(UString const &name)
{
  return XMLParseUtil::attrib(reader, name);
}

UString
LRXCompiler::attrib(UString const &name, const UString fallback)
{
  return XMLParseUtil::attrib(reader, name, fallback);
}

bool
LRXCompiler::allBlanks()
{
  UString text = XMLParseUtil::readValue(reader);
  for (auto& c : text) {
    if (!u_isspace(c)) {
      return false;
    }
  }
  return true;
}

void
LRXCompiler::parse(string const &fitxer)
{
  reader = xmlReaderForFile(fitxer.c_str(), NULL, 0);
  if(reader == NULL)
  {
    cerr << "Error: Cannot open '" << fitxer << "'." << endl;
    exit(EXIT_FAILURE);
  }

  int ret = xmlTextReaderRead(reader);
  while(ret == 1)
  {
    procNode();
    ret = xmlTextReaderRead(reader);
  }

  transducer.minimize();

  if(ret != 0)
  {
    cerr << "Error: Parse error at the end of input." << endl;
  }

}

void
LRXCompiler::procNode()
{
  UString nombre = XMLParseUtil::readName(reader);

  if(nombre == "#text"_u)
  {
    /* ignorar */
  }
  else if(nombre== "#comment"_u)
  {
    /* ignorar */
  }
  else if(nombre == LRX_COMPILER_LRX_ELEM)
  {
    /* ignorar */
  }
  else if(nombre == LRX_COMPILER_DEFSEQS_ELEM)
  {
    /* ignorar */
  }
  else if(nombre == LRX_COMPILER_DEFSEQ_ELEM)
  {
    procDefSeq();
  }
  else if(nombre == LRX_COMPILER_RULES_ELEM)
  {
    /* ignorar */
  }
  else if(nombre == LRX_COMPILER_RULE_ELEM)
  {
    procRule();
  }
  else
  {
    error("Invalid node '<%S>'.", nombre.c_str());
  }

  return;
}

void
LRXCompiler::procRule()
{
  UString comment = this->attrib(LRX_COMPILER_COMMENT_ATTR);
  UString xweight = this->attrib(LRX_COMPILER_WEIGHT_ATTR);
  UString nombre = this->attrib(LRX_COMPILER_NAME_ATTR);
  double weight = LRX_COMPILER_DEFAULT_WEIGHT;
  if (!xweight.empty()) {
    weight = StringUtils::stod(xweight);
  }

  if(weight <= -numeric_limits<int>::max())
  {
    weight = LRX_COMPILER_DEFAULT_WEIGHT ;
  }

  // insert new epsilon:epsilon step at beginning to rule to ensure that it doesn't overlap with any other rules
  currentState = transducer.insertNewSingleTransduction(alphabet(0, 0), currentState);

  currentRuleId++;
  UString ruleId = "<"_u + StringUtils::itoa(currentRuleId) + ">"_u;
  weights[currentRuleId] = weight;

  debug("  rule: %d, weight: %.2f \n", currentRuleId, weight);

  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1) {
      error("Parse error.");
    }

    UString name = XMLParseUtil::readName(reader);
    skipBlanks(name);

    if(name == LRX_COMPILER_MATCH_ELEM)
    {
      procMatch();
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      lastState = currentState;
      procOr();
    }
    else if(name == LRX_COMPILER_REPEAT_ELEM)
    {
      procRepeat();
    }
    else if(name == LRX_COMPILER_SEQ_ELEM)
    {
      procSeq();
    }
    else if(name == LRX_COMPILER_RULE_ELEM)
    {
      currentState = transducer.insertSingleTransduction(word_boundary, currentState);
      if(!alphabet.isSymbolDefined(ruleId.c_str()))
      {
        alphabet.includeSymbol(ruleId.c_str());
      }
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(ruleId.c_str())), currentState);
      transducer.setFinal(currentState);
      currentState = initialState;
      return;
    }
    else
    {
      error("Invalid inclusion of '<%S>' into '<rule>'.", name.c_str());
    }
  }
}

void
LRXCompiler::procOr()
{
  debug("    or: \n");

  int or_initial_state = currentState;
  vector<int> reachedStates;
  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1) {
      error("Parse error.");
    }

    UString name = XMLParseUtil::readName(reader);
    skipBlanks(name);

    if(name == LRX_COMPILER_MATCH_ELEM)
    {
      currentState = transducer.insertNewSingleTransduction(0, or_initial_state);
      procMatch();
      reachedStates.push_back(currentState);
    }
    else if(name == LRX_COMPILER_SEQ_ELEM)
    {
      currentState = transducer.insertNewSingleTransduction(0, or_initial_state);
      procSeq();
      reachedStates.push_back(currentState);
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      if(reachedStates.size() > 1)
      {
        for(auto& it : reachedStates)
        {
          if(it == currentState)
          {
            continue;
          }
          transducer.linkStates(it, currentState, 0);
        }
      }
      break;
    }
    else
    {
      error("Invalid inclusion of '<%S>' into '<or>'.", name.c_str());
    }
  }

  return;
}


void
LRXCompiler::procDefSeq()
{
  canSelect = false;
  Transducer temp = transducer;
  transducer.clear();
  int oldstate = currentState;
  currentState = initialState;
  lastState = initialState;
  UString seqname = this->attrib(LRX_COMPILER_NAME_ATTR);
  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1) {
      error("Parse error.");
    }

    UString name = XMLParseUtil::readName(reader);
    skipBlanks(name);

    if(name == LRX_COMPILER_MATCH_ELEM)
    {
      procMatch();
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      lastState = currentState;
      procOr();
    }
    else if(name == LRX_COMPILER_REPEAT_ELEM)
    {
      procRepeat();
    }
    else if(name == LRX_COMPILER_SEQ_ELEM)
    {
      procSeq();
    }
    else if(name == LRX_COMPILER_DEFSEQ_ELEM)
    {
      transducer.setFinal(currentState);
      break;
    }
    else
    {
      error("Invalid inclusion of '<%S>' into '<repeat>'.", name.c_str());
    }
  }
  sequences[seqname] = transducer;
  currentState = oldstate;
  lastState = oldstate;
  transducer = temp;
  canSelect = true;
}


int add_loop(Transducer* t, int state, int32_t sym, bool star)
{
  if (star) {
    state = t->insertNewSingleTransduction(0, state);
  } else {
    state = t->insertSingleTransduction(sym, state);
  }
  t->linkStates(state, state, sym);
  return state;
}

int add_str(Transducer* t, int state, Alphabet& alpha, const UString& s)
{
  for (auto& c : s) {
    state = t->insertSingleTransduction(alpha(c, 0), state);
  }
  return state;
}


int
LRXCompiler::compileSpecifier(const UString& type, Transducer* t, int state,
                              UString* key)
{
  UString lemma = this->attrib(LRX_COMPILER_LEMMA_ATTR, "*"_u);
  UString suffix = this->attrib(LRX_COMPILER_SUFFIX_ATTR);
  UString contains = this->attrib(LRX_COMPILER_CONTAINS_ATTR);
  UString _case = this->attrib(LRX_COMPILER_CASE_ATTR);
  // case could in principle be non-exclusive with the others

  if ((lemma != "*"_u ? 1 : 0) + (suffix.empty() ? 0 : 1) +
      (contains.empty() ? 0 : 1) + (_case.empty() ? 0 : 1) > 1) {
    error("Only 1 of lemma=, suffix=, contains=, case= is supported on a single element.");
  }

  // for future use
  //UString surface = this->attrib(LRX_COMPILER_SURFACE_ATTR);

  UString tags = this->attrib(LRX_COMPILER_TAGS_ATTR, "*"_u);

  debug("      %S: [%S, %S, %S, %S] %S\n", type.c_str(), lemma.c_str(), suffix.c_str(), contains.c_str(), _case.c_str(), tags.c_str());

  if (!_case.empty()) {
    for (auto& c : _case) {
      if (u_isupper(c)) {
        state = add_loop(t, state, alphabet(any_upper, 0), false);
        if (key) {
          *key = *key + "<ANY_UPPER>"_u;
          currentState = transducer.insertSingleTransduction(alphabet(0, any_upper), currentState);
        }
      } else {
        state = add_loop(t, state, alphabet(any_lower, 0), false);
        if (key) {
          *key = *key + "<ANY_LOWER>"_u;
          currentState = transducer.insertSingleTransduction(alphabet(0, any_lower), currentState);
        }
      }
    }
  } else if (!suffix.empty()) {
    state = add_loop(t, state, alphabet(any_char, 0), true);
    state = add_str(t, state, alphabet, suffix);
    if (key) {
      *key = *key + "<ANY_CHAR>"_u;
      *key = *key + suffix;
      currentState = transducer.insertSingleTransduction(alphabet(0, any_char), currentState);
      for (auto& c : suffix) {
        currentState = transducer.insertSingleTransduction(alphabet(0, c), currentState);
      }
    }
  } else if (!contains.empty()) {
    state = add_loop(t, state, alphabet(any_char, 0), true);
    state = add_str(t, state, alphabet, contains);
    state = add_loop(t, state, alphabet(any_char, 0), true);
    if (key) {
      *key = *key + "<ANY_CHAR>"_u;
      *key = *key + contains;
      *key = *key + "<ANY_CHAR>"_u;
      currentState = transducer.insertSingleTransduction(alphabet(0, any_char), currentState);
      for (auto& c : contains) {
        currentState = transducer.insertSingleTransduction(alphabet(0, c), currentState);
      }
      currentState = transducer.insertSingleTransduction(alphabet(0, any_char), currentState);
    }
  } else if (lemma == "*"_u) {
    state = add_loop(t, state, alphabet(any_char, 0), false);
    if (key) {
      *key = *key + "<ANY_CHAR>"_u;
      currentState = transducer.insertSingleTransduction(alphabet(0, any_char), currentState);
    }
  } else {
    state = add_str(t, state, alphabet, lemma);
    if (key) {
      *key = *key + lemma;
      for (auto& c : lemma) {
        currentState = transducer.insertSingleTransduction(alphabet(0, c), currentState);
      }
    }
  }

  for (auto& it : StringUtils::split(tags, "."_u)) {
    if (it.empty()) {
      continue;
    }
    UString tag = "<"_u + it + ">"_u;
    debug("        tag: %S\n", tag.c_str());
    if (tag == "<*>"_u) {
      state = add_loop(t, state, alphabet(any_tag, 0), true);
      if (key) {
        *key = *key + "<ANY_TAG>"_u;
        currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
      }
    } else {
      if (!alphabet.isSymbolDefined(tag)) {
        alphabet.includeSymbol(tag);
      }
      state = t->insertSingleTransduction(alphabet(alphabet(tag), 0), state);
      if (key) {
        *key = *key + tag;
        currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag)), currentState);
      }
    }
  }

  return state;
}


void
LRXCompiler::procMatch()
{
  currentState = compileSpecifier(LRX_COMPILER_TYPE_MATCH, &transducer,
                                  currentState, nullptr);

  if(xmlTextReaderIsEmptyElement(reader))
  {
    // If self-closing
    currentState = transducer.insertSingleTransduction(word_boundary, currentState);
    currentState = transducer.insertSingleTransduction(alphabet(0, alphabet("<skip>"_u)), currentState);
    return;
  }

  UString name = ""_u;
  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1) {
      error("Parse error.");
    }

    name = XMLParseUtil::readName(reader);
    skipBlanks(name);

    if(name == LRX_COMPILER_SELECT_ELEM)
    {
      if(!canSelect) {
        error("<select> is not permitted inside <repeat>.");
      }
      procSelect();
    }
    else if(name == LRX_COMPILER_REMOVE_ELEM)
    {
      if(!canSelect) {
        error("<remove> is not permitted inside <repeat>.");
      }
      procRemove();
    }
    else if(name == LRX_COMPILER_MATCH_ELEM)
    {
      return;
    }
    else
    {
      error("Invalid inclusion of '<%S>' into '<match>'.");
    }
  }


  return;
}

void
LRXCompiler::procSelect()
{
  UString key = "<"_u + LRX_COMPILER_TYPE_SELECT + ">"_u;
  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();

  currentState = transducer.insertSingleTransduction(word_boundary, currentState);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet("<"_u + LRX_COMPILER_TYPE_SELECT + ">"_u)), currentState);

  localCurrentState = compileSpecifier(LRX_COMPILER_TYPE_SELECT, &recogniser,
                                       localCurrentState, &key);

  recogniser.setFinal(localCurrentState);

  recognisers[key] = recogniser;
  debug("        select: %d\n", recognisers[key].size());
  //currentState = transducer.insertSingleTransduction(word_boundary, currentState);

  return;
}

void
LRXCompiler::procRemove()
{
  UString key = "<"_u + LRX_COMPILER_TYPE_REMOVE + ">"_u;
  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();

  currentState = transducer.insertSingleTransduction(word_boundary, currentState);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet("<"_u + LRX_COMPILER_TYPE_REMOVE + ">"_u)), currentState);

  localCurrentState = compileSpecifier(LRX_COMPILER_TYPE_REMOVE, &recogniser,
                                       localCurrentState, &key);

  recogniser.setFinal(localCurrentState);

  recognisers[key] = recogniser;
  debug("        remove: %d\n", recognisers[key].size());

  return;
}


void
LRXCompiler::procRepeat()
{
  bool couldSelect = canSelect;
  canSelect = false;
  UString xfrom = this->attrib(LRX_COMPILER_FROM_ATTR);
  UString xupto = this->attrib(LRX_COMPILER_UPTO_ATTR);
  int from = StringUtils::stoi(xfrom);
  int upto = StringUtils::stoi(xupto);
  if(from < 0 || upto < 0) {
    error("Number of repetitions cannot be negative.");
  } else if(from > upto) {
    error("Lower bound on number of repetitions cannot be larger than upper bound.");
  }
  int count = upto - from;
  int oldstate = currentState;
  Transducer temp = transducer;
  transducer.clear();
  currentState = initialState;
  lastState = initialState;
  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1) {
      error("Parse error.");
    }

    UString name = XMLParseUtil::readName(reader);
    skipBlanks(name);

    if(name == LRX_COMPILER_MATCH_ELEM)
    {
      procMatch();
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      lastState = currentState;
      procOr();
    }
    else if(name == LRX_COMPILER_SEQ_ELEM)
    {
      procSeq();
    }
    else if(name == LRX_COMPILER_REPEAT_ELEM)
    {
      transducer.setFinal(currentState);
      break;
    }
    else
    {
      error("Invalid inclusion of '<%S>' into '<repeat>'.", name.c_str());
    }
  }
  for(int i = 0; i < from; i++)
  {
    oldstate = temp.insertTransducer(oldstate, transducer);
  }
  transducer.optional();
  for(int i = 0; i < count; i++)
  {
    oldstate = temp.insertTransducer(oldstate, transducer);
  }
  currentState = oldstate;
  lastState = oldstate;
  transducer = temp;
  canSelect = couldSelect;
}


void
LRXCompiler::procSeq()
{
  UString name = this->attrib(LRX_COMPILER_NAME_ATTR);
  if(sequences.find(name) == sequences.end())
  {
    error("Sequence '%S' is not defined.", name.c_str());
  }
  currentState = transducer.insertTransducer(currentState, sequences[name]);
}


void
LRXCompiler::write(FILE *fst)
{
  alphabet.write(fst);

  Compression::multibyte_write(recognisers.size(), fst);
  for(auto& it : recognisers)
  {
    Compression::string_write(it.first, fst);
    debug("+ %d => %S\n", it.second.size(), it.first.c_str());
    if (debugMode) {
      it.second.show(alphabet, debug_output, 0, false);
    }
    it.second.write(fst);
  }

  Compression::string_write("main"_u, fst);
  if(outputGraph)
  {
    transducer.show(alphabet, debug_output, 0, false);
  }
  transducer.write(fst);

  for(auto& it : weights)
  {
    debug("%.4f %d\n", it.second, it.first);
    weight record{it.first, "", it.second};
    weight_to_le(record);
    fwrite((void *)&record, 1, sizeof(weight), fst);
  }

  if(!outputGraph)
  {
    u_fprintf(debug_output, "%d: %d@%d\n", currentRuleId, transducer.size(), transducer.numberOfTransitions());
  }
}
