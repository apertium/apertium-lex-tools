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


void
LRXCompiler::procMatch()
{
  // These are mutually exclusive
  UString lemma = this->attrib(LRX_COMPILER_LEMMA_ATTR, "*"_u);
  UString contains = this->attrib(LRX_COMPILER_SUFFIX_ATTR);
  UString suffix = this->attrib(LRX_COMPILER_CONTAINS_ATTR);
  UString _case = this->attrib(LRX_COMPILER_CASE_ATTR); // This could potentially be non-exclusive

  // This is currently disabled: Future use
  UString surface = this->attrib(LRX_COMPILER_SURFACE_ATTR);

  UString tags = this->attrib(LRX_COMPILER_TAGS_ATTR, "*"_u);

  if(!surface.empty())
  {
    debug("      match: %S\n", surface.c_str());

    for(auto& it : surface)
    {
      currentState = transducer.insertSingleTransduction(alphabet(it, 0), currentState);
    }
  }
  else
  {
    debug("      match: [%S, %S, %S, %S] %S\n", lemma.c_str(), suffix.c_str(), contains.c_str(), _case.c_str(), tags.c_str());

    if(_case != ""_u)
    {
      if(_case == "AA"_u) // <ANY_UPPER>+
      {
        int localLast = currentState;
        currentState = transducer.insertSingleTransduction(alphabet(any_upper, 0), currentState);
        transducer.linkStates(currentState, localLast, 0);
      }
      else if(_case == "aa"_u)  // <ANY_LOWER>+
      {
        int localLast = currentState;
        currentState = transducer.insertSingleTransduction(alphabet(any_lower, 0), currentState);
        transducer.linkStates(currentState, localLast, 0);
      }
      else if(_case == "Aa"_u) // <ANY_UPPER>+ <ANY_LOWER>+
      {
        currentState = transducer.insertSingleTransduction(alphabet(any_upper, 0), currentState);
        int localLast = currentState;
        currentState = transducer.insertSingleTransduction(alphabet(any_lower, 0), currentState);
        transducer.linkStates(currentState, localLast, 0);
      }
    }
    if(lemma == "*"_u && suffix.empty() && contains.empty() && _case.empty())
    {
      // This is only if there is no suffix or case or contains
      debug("        char: -\n");
      int localLast = currentState;
      currentState = transducer.insertSingleTransduction(alphabet(any_char, 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
    }
    else if(suffix != ""_u)
    {
      // A suffix is <ANY_CHAR> any amount of times followed by whatever is in the suffix
      int localLast = currentState;
      currentState = transducer.insertSingleTransduction(alphabet(any_char, 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
      for(auto& it : suffix)
      {
        currentState = transducer.insertSingleTransduction(alphabet(it, 0), currentState);
      }
    }
    else if(!contains.empty())
    {
      // A contains is <ANY_CHAR> any amount of times followed by whatever is in the attribute
      // followed by <ANY_CHAR> any amount of times
      int localLast = currentState;
      currentState = transducer.insertSingleTransduction(alphabet(any_char, 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
      for(auto& it : suffix)
      {
        currentState = transducer.insertSingleTransduction(alphabet(it, 0), currentState);
      }
      currentState = transducer.insertSingleTransduction(alphabet(any_char, 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
    }
    else if(lemma != "*"_u)
    {
      for(auto& it : lemma)
      {
        currentState = transducer.insertSingleTransduction(alphabet(it, 0), currentState);
      }
    }
    else
    {
      cerr << "Something surprising happened in <match> compilation\n";
    }

    UString tag;
    for(auto& it : tags)
    {
      if(it == '.')
      {
        if(tag.empty())
        {
          continue;
        }
        tag = "<"_u + tag + ">"_u;
        if(!alphabet.isSymbolDefined(tag.c_str()))
        {
          alphabet.includeSymbol(tag.c_str());
        }
        debug("        tag: %S\n", tag.c_str());
        if(tag == "<*>"_u)
        {
          int localLast = currentState;
          currentState = transducer.insertSingleTransduction(alphabet(any_tag, 0), currentState);
          transducer.linkStates(currentState, localLast, 0);
        }
        else
        {
          currentState = transducer.insertSingleTransduction(alphabet(alphabet(tag.c_str()), 0), currentState);
        }
        tag = ""_u;
        continue;
      }
      tag = tag + it;
    }
    if(tag == "*"_u)
    {
      debug("        tag: %S\n", tag.c_str());
      int localLast = currentState;
      currentState = transducer.insertSingleTransduction(alphabet(any_tag, 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
    }
    else if(tag.empty())
    {
    }
    else
    {
      tag = "<"_u + tag + ">"_u;
      if(!alphabet.isSymbolDefined(tag.c_str()))
      {
        alphabet.includeSymbol(tag.c_str());
      }
      debug("        tag: %S\n", tag.c_str());
      currentState = transducer.insertSingleTransduction(alphabet(alphabet(tag.c_str()), 0), currentState);
    }
  }

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

  UString lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR, "*"_u);
  UString tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

  UString key = "<"_u + LRX_COMPILER_TYPE_SELECT + ">"_u;
  if(lemma != "*"_u)
  {
    key += lemma;
  }

  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();

  debug("        select: %S, %S\n", lemma.c_str(), tags.c_str());

  currentState = transducer.insertSingleTransduction(word_boundary, currentState);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet("<"_u + LRX_COMPILER_TYPE_SELECT + ">"_u)), currentState);


  if(lemma == "*"_u)
  {
    currentState = transducer.insertSingleTransduction(alphabet(0, any_char), currentState);
    int localLast = localCurrentState;
    localCurrentState = recogniser.insertSingleTransduction(alphabet(any_char ,0), localCurrentState);
    recogniser.linkStates(localCurrentState, localLast, 0);
    key = key + "<ANY_CHAR>"_u;
  }
  else {
    for (auto &it : lemma) {
      currentState = transducer.insertSingleTransduction(alphabet(0, it), currentState);
      localCurrentState = recogniser.insertSingleTransduction(alphabet(it, 0), localCurrentState);
    }
  }

  if(!tags.empty()) {
    UString tag;
    for(auto& it : tags) {
      if(it == '.')
      {
        tag = "<"_u + tag + ">"_u;
        if(!alphabet.isSymbolDefined(tag.c_str()))
        {
          alphabet.includeSymbol(tag.c_str());
        }
        debug("        tag: %S\n", tag.c_str());
        if(tag == "<*>"_u)
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
	  int localLast = localCurrentState;
          localCurrentState = recogniser.insertSingleTransduction(alphabet(any_tag ,0), localCurrentState);
	  recogniser.linkStates(localCurrentState, localLast, 0);
          key = key + "<ANY_TAG>"_u;
        }
        else
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
          localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
          key = key + tag;
        }
        tag = ""_u;
        continue;
      }
      tag = tag + it;
    }
    if(tag == "*"_u)
    {
      debug("        tag: %S\n", tag.c_str());
      currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
      int localLast = localCurrentState;
      localCurrentState = recogniser.insertSingleTransduction(alphabet(any_tag ,0), localCurrentState);
      recogniser.linkStates(localCurrentState, localLast, 0);
      key = key + "<ANY_TAG>"_u;
    }
    else
    {
      tag = "<"_u + tag + ">"_u;
      if(!alphabet.isSymbolDefined(tag.c_str()))
      {
        alphabet.includeSymbol(tag.c_str());
      }
      debug("        tag: %S\n", tag.c_str());
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
      localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
      key = key + tag;
    }
  }
  else
  {
    debug("        tag: -\n");
    currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
    int localLast = localCurrentState;
    localCurrentState = recogniser.insertSingleTransduction(alphabet(any_tag ,0), localCurrentState);
    recogniser.linkStates(localCurrentState, localLast, 0);
    key = key + "<ANY_TAG>"_u;
  }


  recogniser.setFinal(localCurrentState);

  recognisers[key] = recogniser;
  debug("        select: %d\n", recognisers[key].size());
  //currentState = transducer.insertSingleTransduction(word_boundary, currentState);

  return;
}

void
LRXCompiler::procRemove()
{

  UString lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR, "*"_u);
  UString tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

  UString key = "<"_u + LRX_COMPILER_TYPE_REMOVE + ">"_u;
  if(lemma != "*"_u)
  {
    key += lemma;
  }

  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();

  debug("        remove: %S, %S\n", lemma.c_str(), tags.c_str());

  currentState = transducer.insertSingleTransduction(word_boundary, currentState);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet("<"_u + LRX_COMPILER_TYPE_REMOVE + ">"_u)), currentState);

  if(lemma == "*"_u)
  {
    currentState = transducer.insertSingleTransduction(alphabet(0, any_char), currentState);
    int localLast = localCurrentState;
    localCurrentState = recogniser.insertSingleTransduction(alphabet(any_char ,0), localCurrentState);
    recogniser.linkStates(localCurrentState, localLast, 0);
    key = key + "<ANY_CHAR>"_u;
  }
  else
  {
    for(auto& it : lemma)
    {
      currentState = transducer.insertSingleTransduction(alphabet(0, it), currentState);
      localCurrentState = recogniser.insertSingleTransduction(alphabet(it, 0), localCurrentState);
    }
  }

  if(tags != ""_u)
  {
    UString tag = ""_u;
    for(auto& it : tags)
    {
      if(it == '.')
      {
        tag = "<"_u + tag + ">"_u;
        if(!alphabet.isSymbolDefined(tag.c_str()))
        {
          alphabet.includeSymbol(tag.c_str());
        }
        debug("        tag: %S\n", tag.c_str());
        if(tag == "<*>"_u)
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
	  int localLast = localCurrentState;
          localCurrentState = recogniser.insertSingleTransduction(alphabet(any_tag, 0), localCurrentState);
	  recogniser.linkStates(localCurrentState, localLast, 0);
          key = key + "<ANY_TAG>"_u;
        }
        else
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
          localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
          key = key + tag;
        }
        tag = ""_u;
        continue;
      }
      tag = tag + it;
    }
    if(tag == "*"_u)
    {
      debug("        tag: %S\n", tag.c_str());
      currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
      int localLast = localCurrentState;
      localCurrentState = recogniser.insertSingleTransduction(alphabet(any_tag, 0), localCurrentState);
      recogniser.linkStates(localCurrentState, localLast, 0);
      key = key + "<ANY_TAG>"_u;
    }
    else
    {
      tag = "<"_u + tag + ">"_u;
      if(!alphabet.isSymbolDefined(tag.c_str()))
      {
        alphabet.includeSymbol(tag);
      }
      debug("        tag: %S\n", tag.c_str());
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
      localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
      key = key + tag;
    }
  }
  else
  {
    debug("        tag: -\n");
    currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
    int localLast = localCurrentState;
    localCurrentState = recogniser.insertSingleTransduction(alphabet(any_tag,0), localCurrentState);
    recogniser.linkStates(localCurrentState, localLast, 0);
    key = key + "<ANY_TAG>"_u;
  }


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
