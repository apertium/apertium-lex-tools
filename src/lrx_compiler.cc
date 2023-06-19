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
#include <lttoolbox/xml_walk_util.h>
#include <lttoolbox/compression.h>
#include <iostream>
#include <limits>

using namespace std;

UString const LRXCompiler::LRX_COMPILER_LRX_ELEM        = "lrx"_u;
UString const LRXCompiler::LRX_COMPILER_DEFMACROS_ELEM  = "def-macros"_u;
UString const LRXCompiler::LRX_COMPILER_DEFMACRO_ELEM   = "def-macro"_u;
UString const LRXCompiler::LRX_COMPILER_DEFSEQS_ELEM    = "def-seqs"_u;
UString const LRXCompiler::LRX_COMPILER_DEFSEQ_ELEM     = "def-seq"_u;
UString const LRXCompiler::LRX_COMPILER_RULES_ELEM      = "rules"_u;
UString const LRXCompiler::LRX_COMPILER_RULE_ELEM       = "rule"_u;
UString const LRXCompiler::LRX_COMPILER_MACRO_ELEM      = "macro"_u;
UString const LRXCompiler::LRX_COMPILER_MATCH_ELEM      = "match"_u;
UString const LRXCompiler::LRX_COMPILER_SELECT_ELEM     = "select"_u;
UString const LRXCompiler::LRX_COMPILER_REMOVE_ELEM     = "remove"_u;
UString const LRXCompiler::LRX_COMPILER_OR_ELEM         = "or"_u;
UString const LRXCompiler::LRX_COMPILER_REPEAT_ELEM     = "repeat"_u;
UString const LRXCompiler::LRX_COMPILER_SEQ_ELEM        = "seq"_u;
UString const LRXCompiler::LRX_COMPILER_BEGIN_ELEM      = "begin"_u;
UString const LRXCompiler::LRX_COMPILER_PARAM_ELEM      = "param"_u;
UString const LRXCompiler::LRX_COMPILER_WITH_PARAM_ELEM = "with-param"_u;
UString const LRXCompiler::LRX_COMPILER_DEF_SET_ELEM    = "def-set"_u;
UString const LRXCompiler::LRX_COMPILER_SET_ELEM        = "set"_u;
UString const LRXCompiler::LRX_COMPILER_LEMMA_ELEM      = "lemma"_u;

UString const LRXCompiler::LRX_COMPILER_LEMMA_ATTR      = "lemma"_u;
UString const LRXCompiler::LRX_COMPILER_SUFFIX_ATTR     = "suffix"_u;
UString const LRXCompiler::LRX_COMPILER_CONTAINS_ATTR   = "contains"_u;
UString const LRXCompiler::LRX_COMPILER_CASE_ATTR       = "case"_u;
UString const LRXCompiler::LRX_COMPILER_SURFACE_ATTR    = "surface"_u;
UString const LRXCompiler::LRX_COMPILER_TAGS_ATTR       = "tags"_u;
UString const LRXCompiler::LRX_COMPILER_WEIGHT_ATTR     = "weight"_u;
UString const LRXCompiler::LRX_COMPILER_COMMENT_ATTR    = "c"_u;
UString const LRXCompiler::LRX_COMPILER_NAME_ATTR       = "n"_u;
UString const LRXCompiler::LRX_COMPILER_VALUE_ATTR      = "v"_u;
UString const LRXCompiler::LRX_COMPILER_NODES_ATTR      = "nodes"_u;
UString const LRXCompiler::LRX_COMPILER_NPAR_ATTR       = "npar"_u;
UString const LRXCompiler::LRX_COMPILER_FROM_ATTR       = "from"_u;
UString const LRXCompiler::LRX_COMPILER_UPTO_ATTR       = "upto"_u;
UString const LRXCompiler::LRX_COMPILER_GLOB_ATTR       = "glob"_u;
UString const LRXCompiler::LRX_COMPILER_GLOB_PLUS_VAL   = "plus"_u;
UString const LRXCompiler::LRX_COMPILER_GLOB_STAR_VAL   = "star"_u;

UString const LRXCompiler::LRX_COMPILER_TYPE_MATCH      = "match"_u;
UString const LRXCompiler::LRX_COMPILER_TYPE_SELECT     = "select"_u;
UString const LRXCompiler::LRX_COMPILER_TYPE_REMOVE     = "remove"_u;
UString const LRXCompiler::LRX_COMPILER_TYPE_SKIP       = "skip"_u;

UString const LRXCompiler::LRX_COMPILER_SYM_SELECT      = "<select>"_u;
UString const LRXCompiler::LRX_COMPILER_SYM_REMOVE      = "<remove>"_u;
UString const LRXCompiler::LRX_COMPILER_SYM_SKIP        = "<skip>"_u;

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

LRXCompiler::LRXCompiler()
{
  debug_output = u_finit(stderr, NULL, NULL);

  initialState = transducer.getInitial();
  currentState = initialState;

  alphabet.includeSymbol(LRX_COMPILER_SYM_SELECT);
  alphabet.includeSymbol(LRX_COMPILER_SYM_REMOVE);
  alphabet.includeSymbol(LRX_COMPILER_SYM_SKIP);

  alphabet.includeSymbol("<ANY_TAG>"_u);
  alphabet.includeSymbol("<ANY_CHAR>"_u);
  alphabet.includeSymbol("<ANY_UPPER>"_u);
  alphabet.includeSymbol("<ANY_LOWER>"_u);
  alphabet.includeSymbol("<$>"_u);
  alphabet.includeSymbol("<$$>"_u);

  any_tag        = alphabet("<ANY_TAG>"_u);
  any_char       = alphabet("<ANY_CHAR>"_u);
  any_upper      = alphabet("<ANY_UPPER>"_u);
  any_lower      = alphabet("<ANY_LOWER>"_u);
  word_boundary  = alphabet(alphabet("<$>"_u), alphabet("<$>"_u));
  null_boundary  = alphabet(alphabet("<$$>"_u), alphabet("<$$>"_u));
  select_sym     = alphabet(0, alphabet(LRX_COMPILER_SYM_SELECT));
  remove_sym     = alphabet(0, alphabet(LRX_COMPILER_SYM_REMOVE));
  skip_sym       = alphabet(0, alphabet(LRX_COMPILER_SYM_SKIP));
}

LRXCompiler::~LRXCompiler()
{
  for (auto& it : sets) {
    delete it.second.first;
  }
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

UString
name(xmlNode* node)
{
  return to_ustring((const char*) node->name);
}

UString
LRXCompiler::attr(xmlNode* node, const UString& attr, const UString& fallback)
{
  UString id = getattr(node, "p"_u + attr);
  if (!id.empty()) {
    if (!getattr(node, attr).empty()) {
      error_and_die(node, "Cannot provide both regular value and macro value for attribute %S.", attr.c_str());
    }
    int idx = StringUtils::stoi(id);
    if (currentMacro == nullptr) {
      error_and_die(node, "Cannot access macro parameter 'p%S' - not in a macro.", attr.c_str());
    }
    if (idx > macro_string_vars.size() || idx < 1) {
      error_and_die(node, "Parameter index out of range for macro '%S' - %d is not between 1 and %d.", getattr(currentMacro, LRX_COMPILER_NAME_ATTR).c_str(), idx, macro_string_vars.size());
    }
    return macro_string_vars[idx-1];
  } else {
    return getattr(node, attr, fallback);
  }
}

UString
LRXCompiler::attr(xmlNode* node, const UString& attr)
{
  return this->attr(node, attr, ""_u);
}

void
LRXCompiler::parse(string const &fitxer)
{
  procNode(load_xml(fitxer.c_str()));
  transducer.minimize();
}

void
LRXCompiler::compileSequence(xmlNode* node)
{
  UString outer_name = name(node);
  for (auto ch : children(node)) {
    UString inner_name = name(ch);
    if (inner_name == LRX_COMPILER_BEGIN_ELEM) {
      currentState = transducer.insertSingleTransduction(null_boundary, currentState);
    } else if (inner_name == LRX_COMPILER_MATCH_ELEM) {
      procMatch(ch);
    } else if (inner_name == LRX_COMPILER_OR_ELEM) {
      procOr(ch);
    } else if (inner_name == LRX_COMPILER_REPEAT_ELEM) {
      procRepeat(ch);
    } else if (inner_name == LRX_COMPILER_SEQ_ELEM) {
      procSeq(ch);
    } else if (inner_name == LRX_COMPILER_SET_ELEM) {
      procSet(ch);
    } else if (inner_name == LRX_COMPILER_PARAM_ELEM) {
      int idx = StringUtils::stoi(getattr(ch, LRX_COMPILER_NAME_ATTR));
      if (idx > macro_node_vars.size() || idx < 1) {
        if (currentMacro == nullptr) {
          error_and_die(ch, "Cannot use <param> outside of <def-macro>.");
        } else {
          error_and_die(ch, "Parameter index %d out of range for macro '%S' (0-%d).", idx, getattr(currentMacro, LRX_COMPILER_NAME_ATTR).c_str(), macro_node_vars.size());
        }
      }
      compileSequence(macro_node_vars[idx-1]);
    } else {
      error_and_die(ch, "Invalid inclusion of '<%S>' into '<%S>'.", inner_name.c_str(), outer_name.c_str());
    }
  }
}

void
LRXCompiler::procNode(xmlNode* node)
{
  UString nombre = name(node);
  if (nombre == LRX_COMPILER_LRX_ELEM || nombre == LRX_COMPILER_RULES_ELEM) {
    if (getattr(node, LRX_COMPILER_GLOB_ATTR, LRX_COMPILER_GLOB_PLUS_VAL) == LRX_COMPILER_GLOB_STAR_VAL) {
      globIsStar = true;
    }
    for (auto ch : children(node)) procNode(ch);
  } else if (nombre == LRX_COMPILER_DEFSEQS_ELEM || nombre == LRX_COMPILER_DEFMACROS_ELEM) {
    for (auto ch : children(node)) procNode(ch);
  } else if (nombre == LRX_COMPILER_DEFSEQ_ELEM) {
    procDefSeq(node);
  } else if (nombre == LRX_COMPILER_DEF_SET_ELEM) {
    procDefSet(node);
  } else if (nombre == LRX_COMPILER_DEFMACRO_ELEM) {
    UString macname = attr(node, LRX_COMPILER_NAME_ATTR);
    if (macname.empty()) {
      error_and_die(node, "Macro is missing name.");
    } else if (macros.find(macname) != macros.end()) {
      error_and_die(node, "Macro '%S' is defined multiple times.", macname.c_str());
    } else {
      macros[macname] = node;
    }
  } else if (nombre == LRX_COMPILER_RULE_ELEM) {
    procRule(node);
  } else if (nombre == LRX_COMPILER_MACRO_ELEM) {
    procMacro(node);
  } else {
    error_and_die(node, "Invalid node '<%S>'.", nombre.c_str());
  }
}

void
LRXCompiler::procRule(xmlNode* node)
{
  UString xweight = attr(node, LRX_COMPILER_WEIGHT_ATTR);
  UString nombre = attr(node, LRX_COMPILER_NAME_ATTR);
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

  compileSequence(node);
  currentState = transducer.insertSingleTransduction(word_boundary, currentState);
  alphabet.includeSymbol(ruleId);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(ruleId)), currentState);
  transducer.setFinal(currentState);
  currentState = initialState;
}

void
LRXCompiler::procOr(xmlNode* node)
{
  debug("    or: \n");

  int or_initial_state = currentState;
  vector<int> reachedStates;
  for (auto ch : children(node)) {
    UString nombre = name(ch);

    currentState = transducer.insertNewSingleTransduction(0, or_initial_state);

    if (nombre == LRX_COMPILER_MATCH_ELEM) {
      procMatch(ch);
    } else if (nombre == LRX_COMPILER_OR_ELEM) {
      procOr(ch);
    } else if (nombre == LRX_COMPILER_SEQ_ELEM) {
      procSeq(ch);
    } else if (nombre == LRX_COMPILER_SET_ELEM) {
      procSet(ch);
    } else if (nombre == LRX_COMPILER_REPEAT_ELEM) {
      procRepeat(ch);
    }

    reachedStates.push_back(currentState);
  }
  if (reachedStates.size() > 1) {
    for (auto& it : reachedStates) {
      if (it != currentState) {
        transducer.linkStates(it, currentState, 0);
      }
    }
  }
}


void
LRXCompiler::procDefSeq(xmlNode* node)
{
  Transducer temp = transducer;
  transducer.clear();
  int oldstate = currentState;
  currentState = initialState;
  UString seqname = attr(node, LRX_COMPILER_NAME_ATTR);
  compileSequence(node);
  transducer.setFinal(currentState);
  sequences[seqname] = transducer;
  currentState = oldstate;
  transducer = temp;
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
LRXCompiler::compileSpecifier(xmlNode* node, Transducer* t, int state,
                              UString* key)
{
  UString lemma = attr(node, LRX_COMPILER_LEMMA_ATTR, "*"_u);
  UString suffix = attr(node, LRX_COMPILER_SUFFIX_ATTR);
  UString contains = attr(node, LRX_COMPILER_CONTAINS_ATTR);
  UString _case = attr(node, LRX_COMPILER_CASE_ATTR);
  // case could in principle be non-exclusive with the others

  if ((lemma != "*"_u ? 1 : 0) + (suffix.empty() ? 0 : 1) +
      (contains.empty() ? 0 : 1) + (_case.empty() ? 0 : 1) > 1) {
    error_and_die(node, "Only 1 of lemma=, suffix=, contains=, case= is supported on a single element.");
  }

  // for future use
  //UString surface = attr(node, LRX_COMPILER_SURFACE_ATTR);

  UString tags = attr(node, LRX_COMPILER_TAGS_ATTR, "*"_u);

  debug("      %S: [%S, %S, %S, %S] %S\n", name(node).c_str(), lemma.c_str(), suffix.c_str(), contains.c_str(), _case.c_str(), tags.c_str());

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
    if ((!globIsStar && tag == "<*>"_u) ||
        tag == "<+>"_u) {
      state = add_loop(t, state, alphabet(any_tag, 0), false);
      if (key) {
        *key = *key + "<ANY_TAG>"_u;
        currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
      }
    } else if (globIsStar && tag == "<*>"_u) {
      state = add_loop(t, state, alphabet(any_tag, 0), true);
      if (key) {
        *key = *key + "<ANY_TAG>"_u;
        currentState = transducer.insertSingleTransduction(alphabet(0, any_tag), currentState);
      }
    } else if (tag == "<?>"_u) {
      state = t->insertSingleTransduction(alphabet(any_tag, 0), state);
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
LRXCompiler::procMatch(xmlNode* node)
{
  currentState = compileSpecifier(node, &transducer, currentState, nullptr);
  currentState = transducer.insertSingleTransduction(word_boundary, currentState);

  bool empty = true;
  UString nombre;
  for (auto ch : children(node)) {
    nombre = name(ch);
    if (nombre == LRX_COMPILER_SELECT_ELEM || nombre == LRX_COMPILER_REMOVE_ELEM) {
      procSelectRemove(ch);
    } else {
      error_and_die(ch, "Invalid inclusion of '<%S>' into <match>'.", nombre.c_str());
    }
    empty = false;
  }
  if (empty) {
    currentState = transducer.insertSingleTransduction(skip_sym, currentState);
  }
}


void
LRXCompiler::procSelectRemove(xmlNode* node)
{
  bool select = (name(node) == LRX_COMPILER_SELECT_ELEM);
  UString key = (select ? LRX_COMPILER_SYM_SELECT : LRX_COMPILER_SYM_REMOVE);
  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();
  currentState = transducer.insertSingleTransduction((select ? select_sym : remove_sym), currentState);

  localCurrentState = compileSpecifier(node, &recogniser,
                                       localCurrentState, &key);

  recogniser.setFinal(localCurrentState);

  recognisers[key] = recogniser;
  debug("        %S: %d\n", name(node).c_str(), recognisers[key].size());
}


void
LRXCompiler::procRepeat(xmlNode* node)
{
  UString xfrom = attr(node, LRX_COMPILER_FROM_ATTR);
  UString xupto = attr(node, LRX_COMPILER_UPTO_ATTR);
  int from = StringUtils::stoi(xfrom);
  int upto = StringUtils::stoi(xupto);
  if(from < 0 || upto < 0) {
    error_and_die(node, "Number of repetitions cannot be negative.");
  } else if(from > upto) {
    error_and_die(node, "Lower bound on number of repetitions cannot be larger than upper bound.");
  }
  int count = upto - from;
  int oldstate = currentState;
  Transducer temp = transducer;
  transducer.clear();
  currentState = initialState;
  compileSequence(node);
  transducer.setFinal(currentState);
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
  transducer = temp;
}


void
LRXCompiler::procSeq(xmlNode* node)
{
  UString name = attr(node, LRX_COMPILER_NAME_ATTR);
  if(sequences.find(name) == sequences.end())
  {
    error_and_die(node, "Sequence '%S' is not defined.", name.c_str());
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

void
LRXCompiler::procMacro(xmlNode* node)
{
  UString macname = attr(node, LRX_COMPILER_NAME_ATTR);
  if (macros.find(macname) == macros.end()) {
    error_and_die(node, "Unknown macro '%S'.", macname.c_str());
  }
  xmlNode* prevMacro = currentMacro;
  xmlNode* nextMacro = macros[macname];
  vector<UString> current_strings;
  vector<xmlNode*> current_nodes;
  int nodes = StringUtils::stoi(getattr(nextMacro, LRX_COMPILER_NODES_ATTR, "0"_u));
  int npar = StringUtils::stoi(getattr(nextMacro, LRX_COMPILER_NPAR_ATTR, "0"_u));
  for (auto ch : children(node)) {
    if (name(ch) != LRX_COMPILER_WITH_PARAM_ELEM) {
      error_and_die(ch, "Unexpected inclusion of <%S> in <macro>.", name(ch).c_str());
    }
    UString val = attr(ch, LRX_COMPILER_VALUE_ATTR);
    if (val.empty()) {
      current_nodes.push_back(ch);
    } else {
      current_strings.push_back(val);
    }
  }
  if (current_nodes.size() != nodes) {
    error_and_die(node, "Macro '%S' expects %d node parameters, but %d were given.", macname.c_str(), nodes, current_nodes.size());
  }
  if (current_strings.size() != npar) {
    error_and_die(node, "Macro '%S' expects %d string parameters, but %d were given.", macname.c_str(), npar, current_strings.size());
  }
  current_strings.swap(macro_string_vars);
  current_nodes.swap(macro_node_vars);
  currentMacro = nextMacro;
  for (auto ch : children(currentMacro)) procNode(ch);
  currentMacro = prevMacro;
  current_strings.swap(macro_string_vars);
  current_nodes.swap(macro_node_vars);
}

void
LRXCompiler::procSet(xmlNode* node)
{
  UString name = attr(node, LRX_COMPILER_NAME_ATTR);
  auto loc = sets.find(name);
  if (loc == sets.end()) {
    error_and_die(node, "Undefined set %S.", name.c_str());
  }
  currentState = transducer.insertTransducer(currentState, *(loc->second.first));
  UString tags = attr(node, LRX_COMPILER_TAGS_ATTR, loc->second.second);
  for (auto& it : StringUtils::split(tags, "."_u)) {
    if (it.empty()) continue;
    UString tag = "<"_u + it + ">"_u;
    if ((!globIsStar && tag == "<*>"_u) ||
        tag == "<+>"_u) {
      currentState = add_loop(&transducer, currentState, alphabet(any_tag, 0), false);
    } else if (globIsStar && tag == "<*>"_u) {
      currentState = add_loop(&transducer, currentState, alphabet(any_tag, 0), true);
    } else if (tag == "<?>"_u) {
      currentState = transducer.insertSingleTransduction(alphabet(any_tag, 0), currentState);
    } else {
      if (!alphabet.isSymbolDefined(tag)) {
        alphabet.includeSymbol(tag);
      }
      currentState = transducer.insertSingleTransduction(alphabet(alphabet(tag), 0), currentState);
    }
  }
  currentState = transducer.insertSingleTransduction(word_boundary, currentState);
  currentState = transducer.insertSingleTransduction(skip_sym, currentState);
}

void
LRXCompiler::procDefSet(xmlNode* node)
{
  Transducer* t = new Transducer();
  for (auto ch : children(node)) {
    if (name(ch) != LRX_COMPILER_LEMMA_ELEM) continue;
    t->setFinal(add_str(t, 0, alphabet, to_ustring((const char*) xmlNodeGetContent(ch))));
  }
  UString tags = attr(node, LRX_COMPILER_TAGS_ATTR, "*"_u);
  sets[attr(node, LRX_COMPILER_NAME_ATTR)] = std::make_pair(t, tags);
}
