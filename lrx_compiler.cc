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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <lrx_compiler.h>
//#include <lrx_config.h>


using namespace std;

wstring const LRXCompiler::LRX_COMPILER_RULES_ELEM      = L"rules";
wstring const LRXCompiler::LRX_COMPILER_RULE_ELEM       = L"rule";
wstring const LRXCompiler::LRX_COMPILER_MATCH_ELEM      = L"match";
wstring const LRXCompiler::LRX_COMPILER_SELECT_ELEM     = L"select";
wstring const LRXCompiler::LRX_COMPILER_REMOVE_ELEM     = L"remove";
wstring const LRXCompiler::LRX_COMPILER_OR_ELEM         = L"or";
wstring const LRXCompiler::LRX_COMPILER_LEMMA_ATTR      = L"lemma";
wstring const LRXCompiler::LRX_COMPILER_SURFACE_ATTR    = L"surface";
wstring const LRXCompiler::LRX_COMPILER_TAGS_ATTR       = L"tags";
wstring const LRXCompiler::LRX_COMPILER_WEIGHT_ATTR     = L"weight";
wstring const LRXCompiler::LRX_COMPILER_COMMENT_ATTR    = L"c";
wstring const LRXCompiler::LRX_COMPILER_NAME_ATTR       = L"n";

wstring const LRXCompiler::LRX_COMPILER_TYPE_SELECT     = L"select";
wstring const LRXCompiler::LRX_COMPILER_TYPE_REMOVE     = L"remove";
wstring const LRXCompiler::LRX_COMPILER_TYPE_SKIP       = L"skip";

double const  LRXCompiler::LRX_COMPILER_DEFAULT_WEIGHT  = 1.0;

wstring
LRXCompiler::itow(int i)
{
  // Convert an int to a wstring
  wchar_t buf[50];
  memset(buf, '\0', sizeof(buf));
  swprintf(buf, 50, L"%d", i);
  wstring id(buf);
  return id;
}

int 
LRXCompiler::wtoi(wstring w)
{
  // Convert a wstring to an int
  wistringstream wstrm(w);
  int i_name = -numeric_limits<int>::max();
  wstrm >> i_name;

  return i_name;
}

double
LRXCompiler::wtod(wstring w)
{
  // Convert a wstring to a double
  wistringstream wstrm(w);
  double d_name = -numeric_limits<double>::max();
  wstrm >> d_name;

  return d_name;
}

LRXCompiler::LRXCompiler()
{
  LtLocale::tryToSetLocale();

  debugMode = false;
  outputGraph = false;

  currentRuleId = 0;

  initialState = transducer.getInitial();
  currentState = initialState;
  lastState = initialState;

  alphabet.includeSymbol(L"<"+ LRX_COMPILER_TYPE_SELECT + L">");
  alphabet.includeSymbol(L"<"+ LRX_COMPILER_TYPE_REMOVE + L">");
  alphabet.includeSymbol(L"<"+ LRX_COMPILER_TYPE_SKIP + L">");

  alphabet.includeSymbol(L"<ANY_TAG>");
  alphabet.includeSymbol(L"<ANY_CHAR>");
  alphabet.includeSymbol(L"<$>");

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
LRXCompiler::skipBlanks(wstring &name)
{
  while(name == L"#text" || name == L"#comment")
  {
    if(name != L"#comment")
    {
      if(!allBlanks())
      {
        wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
        wcerr << L"): Invalid construction." << endl;
        exit(EXIT_FAILURE);
      }
    }

    xmlTextReaderRead(reader);
    name = XMLParseUtil::towstring(xmlTextReaderConstName(reader));
  }
}

wstring
LRXCompiler::attrib(wstring const &name)
{
  return XMLParseUtil::attrib(reader, name);
}

bool
LRXCompiler::allBlanks()
{
  bool flag = true;
  wstring text = XMLParseUtil::towstring(xmlTextReaderConstValue(reader));

  for(unsigned int i = 0, limit = text.size(); i < limit; i++)
  {
    flag = flag && iswspace(text[i]);
  }

  return flag;
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

  if(ret != 0)
  {
    wcerr << L"Error: Parse error at the end of input." << endl;
  }

}

void
LRXCompiler::procNode()
{
  xmlChar const *xnombre = xmlTextReaderConstName(reader);
  wstring nombre = XMLParseUtil::towstring(xnombre);
  
  if(nombre == L"#text")
  {
    /* ignorar */
  }
  else if(nombre== L"#comment")
  {
    /* ignorar */
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
    wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
    wcerr << L"): Invalid node '<" << nombre << L">'." << endl;
    exit(EXIT_FAILURE);
  }

  return;
}

void
LRXCompiler::procRule()
{
  wstring comment = this->attrib(LRX_COMPILER_COMMENT_ATTR);
  wstring xweight = this->attrib(LRX_COMPILER_WEIGHT_ATTR);
  wstring nombre = this->attrib(LRX_COMPILER_NAME_ATTR);
  double weight =  wtod (xweight);

  if(weight <= -numeric_limits<int>::max()) 
  {
    weight = LRX_COMPILER_DEFAULT_WEIGHT ;
  }



  currentRuleId++;
  wstring ruleId = L"<" + itow(currentRuleId) + L">";
  weights[currentRuleId] = weight;

  if(debugMode)
  {
    fwprintf(stderr, L"  rule: %d, weight: %.2f \n", currentRuleId, weight);
  }

  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1)
    {
      cerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      cerr << L"): Parse error." << endl;
      exit(EXIT_FAILURE);
    }

    wstring name = XMLParseUtil::towstring(xmlTextReaderConstName(reader));
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
    else if(name == LRX_COMPILER_RULE_ELEM)
    {
      currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<$>"), alphabet(L"<$>")), currentState);
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
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_RULE_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  }


  return;
}

void
LRXCompiler::procOr()
{

  if(debugMode)
  {
    fwprintf(stderr, L"    or: \n");
  }

  vector<int> reachedStates;
  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1)
    {
      cerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      cerr << L"): Parse error." << endl;
      exit(EXIT_FAILURE);
    }

    wstring name = XMLParseUtil::towstring(xmlTextReaderConstName(reader));
    skipBlanks(name);

    if(name == LRX_COMPILER_MATCH_ELEM)
    {
      currentState = lastState;
      procMatch();
      reachedStates.push_back(currentState); 
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      if(reachedStates.size() > 1) 
      {
        for(vector<int>::iterator it = reachedStates.begin(); it != reachedStates.end(); it++)
        {
          if(*it == currentState)
          {
            continue;
          }
          transducer.linkStates(*it, currentState, 0);
        }
      }
      break;
    }
    else
    {
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_OR_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  }

  return;
}


void
LRXCompiler::procMatch()
{
  wstring lemma = this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags = this->attrib(LRX_COMPILER_TAGS_ATTR);
  wstring surface = this->attrib(LRX_COMPILER_SURFACE_ATTR);

  if(surface != L"")
  {
    if(debugMode)
    {
      fwprintf(stderr, L"      match: %S\n", surface.c_str());
    }

    for(wstring::iterator it = surface.begin(); it != surface.end(); it++)
    {
      currentState = transducer.insertSingleTransduction(alphabet(*it, 0), currentState);
    }
  }
  else
  {
    if(debugMode)
    {
      fwprintf(stderr, L"      match: %S, %S\n", lemma.c_str(), tags.c_str());
    }


    if(lemma == L"*") {
      lemma = L"";
    }

    if(lemma == L"")
    {
      if(debugMode)
      {
        fwprintf(stderr, L"        char: -\n");
      }
      int localLast = currentState;
      currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<ANY_CHAR>"), 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
    }
    else 
    {
      for(wstring::iterator it = lemma.begin(); it != lemma.end(); it++)
      {
        currentState = transducer.insertSingleTransduction(alphabet(*it, 0), currentState);
      }
    }

    if(tags != L"")
    {
      wstring tag = L"";
      for(wstring::iterator it = tags.begin(); it != tags.end(); it++)
      {
        if(*it == L'.') 
        {
          tag = L"<" + tag + L">";
          if(!alphabet.isSymbolDefined(tag.c_str()))
          { 
            alphabet.includeSymbol(tag.c_str());
          }
          if(debugMode)
          {
            fwprintf(stderr, L"        tag: %S\n", tag.c_str());
          }
          if(tag == L"<*>")
          {
            currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"), 0), currentState);
          }
          else
          {
            currentState = transducer.insertSingleTransduction(alphabet(alphabet(tag.c_str()), 0), currentState);
          }
          tag = L"";
          continue;
        }
        tag = tag + *it;
      }
      if(tag == L"*")
      {
        if(debugMode) 
        {
          fwprintf(stderr, L"        tag: %S\n", tag.c_str());
        }
        int localLast = currentState;
        currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"), 0), currentState);
        transducer.linkStates(currentState, localLast, 0);
      }  
      else 
      {
        tag = L"<" + tag + L">";
        if(!alphabet.isSymbolDefined(tag.c_str()))
        { 
          alphabet.includeSymbol(tag.c_str());
        }
        if(debugMode)
        {
          fwprintf(stderr, L"        tag: %S\n", tag.c_str());
        }
        currentState = transducer.insertSingleTransduction(alphabet(alphabet(tag.c_str()), 0), currentState);
      }
    } 
    else
    {
      if(debugMode)
      {
        fwprintf(stderr, L"        tag: -\n");
      }
      int localLast = currentState;
      currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"), 0), currentState);
      transducer.linkStates(currentState, localLast, 0);
    }
  }

  if(xmlTextReaderIsEmptyElement(reader))
  {
    // If self-closing
    currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<$>"), alphabet(L"<$>")), currentState);
    currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<skip>")), currentState);
    return;
  }

  wstring name = L"";
  while(true)
  {
    int ret = xmlTextReaderRead(reader);
    if(ret != 1)
    {
      cerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      cerr << L"): Parse error." << endl;
      exit(EXIT_FAILURE);
    }

    name = XMLParseUtil::towstring(xmlTextReaderConstName(reader));
    skipBlanks(name);

    if(name == LRX_COMPILER_SELECT_ELEM)
    {
      procSelect();
    }
    else if(name == LRX_COMPILER_REMOVE_ELEM)
    {
      procRemove();
    }
    else if(name == LRX_COMPILER_MATCH_ELEM)
    {
      return;
    }
    else
    {
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_MATCH_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  }  


  return;
}

void
LRXCompiler::procSelect()
{

  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

  wstring key = L"<" + LRX_COMPILER_TYPE_SELECT + L">" + lemma;

  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();

  if(debugMode)
  {
    fwprintf(stderr, L"        select: %S, %S\n", lemma.c_str(), tags.c_str());
  }

  currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<$>"), alphabet(L"<$>")), currentState);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<" + LRX_COMPILER_TYPE_SELECT + L">")), currentState);

  for(wstring::iterator it = lemma.begin(); it != lemma.end(); it++)
  {
    currentState = transducer.insertSingleTransduction(alphabet(0, *it), currentState);
    localCurrentState = recogniser.insertSingleTransduction(alphabet(*it, 0), localCurrentState);
  }

  if(tags != L"")
  {
    wstring tag = L"";
    for(wstring::iterator it = tags.begin(); it != tags.end(); it++)
    {
      if(*it == L'.') 
      {
        tag = L"<" + tag + L">";
        if(!alphabet.isSymbolDefined(tag.c_str()))
        { 
          alphabet.includeSymbol(tag.c_str());
        }
        if(debugMode) 
        {
          fwprintf(stderr, L"        tag: %S\n", tag.c_str());
        }
        if(tag == L"<*>")
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<ANY_TAG>")), currentState);
          localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"),0), localCurrentState);
          key = key + L"<ANY_TAG>";
        }
        else
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
          localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
          key = key + tag;
        }
        tag = L"";
        continue;
      }
      tag = tag + *it;
    }
    if(tag == L"*")
    {
      if(debugMode)
      {
        fwprintf(stderr, L"        tag: %S\n", tag.c_str());
      }
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<ANY_TAG>")), currentState);
      int localLast = localCurrentState;
      localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"),0), localCurrentState);
      recogniser.linkStates(localCurrentState, localLast, 0);
      key = key + L"<ANY_TAG>";
    }  
    else 
    {
      tag = L"<" + tag + L">";
      if(!alphabet.isSymbolDefined(tag.c_str()))
      { 
        alphabet.includeSymbol(tag.c_str());
      }
      if(debugMode)
      {
        fwprintf(stderr, L"        tag: %S\n", tag.c_str());
      }
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
      localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
      key = key + tag;
    }
  } 
  else
  {
    if(debugMode) 
    {
      fwprintf(stderr, L"        tag: -\n");
    }
    currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<ANY_TAG>")), currentState);
    int localLast = localCurrentState;
    localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"),0), localCurrentState);
    recogniser.linkStates(localCurrentState, localLast, 0);
    key = key + L"<ANY_TAG>";
  }
 

  recogniser.setFinal(localCurrentState);

  recognisers[key] = recogniser;
  if(debugMode) 
  {
    fwprintf(stderr, L"        select: %d\n", recognisers[key].size());
  }
  //currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<$>"), alphabet(L"<$>")), currentState);

  return;
}

void
LRXCompiler::procRemove()
{

  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

  wstring key = L"<" + LRX_COMPILER_TYPE_REMOVE + L">" + lemma;

  Transducer recogniser;
  int localCurrentState = recogniser.getInitial();

  if(debugMode)
  {
    fwprintf(stderr, L"        remove: %S, %S\n", lemma.c_str(), tags.c_str());
  }

  currentState = transducer.insertSingleTransduction(alphabet(alphabet(L"<$>"), alphabet(L"<$>")), currentState);
  currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<" + LRX_COMPILER_TYPE_REMOVE + L">")), currentState);

  for(wstring::iterator it = lemma.begin(); it != lemma.end(); it++)
  {
    currentState = transducer.insertSingleTransduction(alphabet(0, *it), currentState);
    localCurrentState = recogniser.insertSingleTransduction(alphabet(*it, 0), localCurrentState);
  }

  if(tags != L"")
  {
    wstring tag = L"";
    for(wstring::iterator it = tags.begin(); it != tags.end(); it++)
    {
      if(*it == L'.') 
      {
        tag = L"<" + tag + L">";
        if(!alphabet.isSymbolDefined(tag.c_str()))
        { 
          alphabet.includeSymbol(tag.c_str());
        }
        if(debugMode) 
        {
          fwprintf(stderr, L"        tag: %S\n", tag.c_str());
        }
        if(tag == L"<*>")
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<ANY_TAG>")), currentState);
          localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"),0), localCurrentState);
          key = key + L"<ANY_TAG>";
        }
        else
        {
          currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
          localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
          key = key + tag;
        }
        tag = L"";
        continue;
      }
      tag = tag + *it;
    }
    if(tag == L"*")
    {
      if(debugMode)
      {
        fwprintf(stderr, L"        tag: %S\n", tag.c_str());
      }
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<ANY_TAG>")), currentState);
      int localLast = localCurrentState;
      localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"),0), localCurrentState);
      recogniser.linkStates(localCurrentState, localLast, 0);
      key = key + L"<ANY_TAG>";
    }  
    else 
    {
      tag = L"<" + tag + L">";
      if(!alphabet.isSymbolDefined(tag.c_str()))
      { 
        alphabet.includeSymbol(tag.c_str());
      }
      if(debugMode)
      {
        fwprintf(stderr, L"        tag: %S\n", tag.c_str());
      }
      currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(tag.c_str())), currentState);
      localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(tag.c_str()),0), localCurrentState);
      key = key + tag;
    }
  } 
  else
  {
    if(debugMode) 
    {
      fwprintf(stderr, L"        tag: -\n");
    }
    currentState = transducer.insertSingleTransduction(alphabet(0, alphabet(L"<ANY_TAG>")), currentState);
    int localLast = localCurrentState;
    localCurrentState = recogniser.insertSingleTransduction(alphabet(alphabet(L"<ANY_TAG>"),0), localCurrentState);
    recogniser.linkStates(localCurrentState, localLast, 0);
    key = key + L"<ANY_TAG>";
  }
 

  recogniser.setFinal(localCurrentState);

  recognisers[key] = recogniser;
  if(debugMode) 
  {
    fwprintf(stderr, L"        remove: %d\n", recognisers[key].size());
  }

  return;
}


void
LRXCompiler::write(FILE *fst)
{
  alphabet.write(fst);

  Compression::multibyte_write(recognisers.size(), fst);
  for(map<wstring, Transducer>::iterator it = recognisers.begin(); it != recognisers.end(); it++) 
  {
    Compression::wstring_write(it->first, fst);
    if(debugMode)
    {
      fwprintf(stderr, L"+ %d => %S\n", it->second.size(), it->first.c_str());
      it->second.show(alphabet, stderr);
    }
    it->second.write(fst);
  }

  Compression::wstring_write(L"main", fst);
  if(outputGraph)
  {
    transducer.show(alphabet, stderr);
  }
  transducer.write(fst);

  struct weight {
        int id;
        double pisu;
  };

  for(map<int, double>::iterator it = weights.begin(); it != weights.end(); it++) 
  {
    if(debugMode)
    {
      fwprintf(stderr, L"%.4f %d\n", it->second, it->first);
    } 
    weight record = {it->first, it->second};
    fwrite((void *)&record, 1, sizeof(weight), fst);
  } 

  if(!outputGraph)
  {
    fwprintf(stderr, L"%d: %d@%d\n", currentRuleId, transducer.size(), transducer.numberOfTransitions());
  }
}
