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

#include <lrx_compiler.h>
#include <lrx_config.h>


using namespace std;

wstring const LRXCompiler::LRX_COMPILER_RULES_ELEM      = L"rules";
wstring const LRXCompiler::LRX_COMPILER_RULE_ELEM       = L"rule";
wstring const LRXCompiler::LRX_COMPILER_MATCH_ELEM      = L"match";
wstring const LRXCompiler::LRX_COMPILER_SELECT_ELEM     = L"select";
wstring const LRXCompiler::LRX_COMPILER_REMOVE_ELEM     = L"remove";
wstring const LRXCompiler::LRX_COMPILER_OR_ELEM         = L"or";
wstring const LRXCompiler::LRX_COMPILER_LEMMA_ATTR      = L"lemma";
wstring const LRXCompiler::LRX_COMPILER_TAGS_ATTR       = L"tags";
wstring const LRXCompiler::LRX_COMPILER_C_ATTR          = L"c";

//wstring const LRXCompiler::LRX_COMPILER_ASTERISK        = L"[0-9A-Za-zÀ-Þà-ÿĀ-Žā-žА-Фа-ф <>@\\+]*";
wstring const LRXCompiler::LRX_COMPILER_ASTERISK        = L"[0-9A-Za-zà-ÿ <>@←→\\+]*";

wstring const LRXCompiler::LRX_COMPILER_TYPE_SELECT     = L"select";
wstring const LRXCompiler::LRX_COMPILER_TYPE_REMOVE     = L"remove";

double const  LRXCompiler::LRX_COMPILER_DEFAULT_WEIGHT   = 1.0;

LRXCompiler::LRXCompiler()
{
  LtLocale::tryToSetLocale();
  current_rule_id = 0;
  current_rule_len = 0;
  current_context_pos = 0;
  outputGraph = false;
}

LRXCompiler::~LRXCompiler()
{
}

void
LRXCompiler::setOutputGraph(bool o)
{
  outputGraph = o;
}


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
  int i_name = -655;
  wstrm >> i_name;

  return i_name;
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

  // At this point we've read the XML file into the rule structures, and now
  // we process the rules structures into the necessary transducers

  for(map<int, LSRule>::iterator it = rules.begin(); it != rules.end(); it++)
  {
    LSRule rule = it->second; 
    fwprintf(stderr, L"\b\b\b\b\b\b\b%d", rule.id);
    fflush(stderr);
    int s = transducer.getInitial();
    wstring w_id = itow(rule.id);

    if(!alphabet.isSymbolDefined(w_id.c_str()))
    {
      alphabet.includeSymbol(w_id.c_str());
    }

    //fwprintf(stderr, L"rule %S (line %d) (len %d) (ops %d):\n", w_id.c_str(), rule.line, rule.len, rule.ops);
    for(map<int, vector<wstring> >::iterator it3 = rule.sl_context.begin(); it3 != rule.sl_context.end(); it3++) 
    {
      int pos = it3->first;
      vector<wstring> sl_patterns = it3->second;

      if(rule.tl_context[pos].size() > 0 && sl_patterns.size() == 1) 
      {
        int k = s;
        for(vector<wstring>::iterator it4 = rule.tl_context[pos].begin(); it4 != rule.tl_context[pos].end(); it4++)
        { 
          wstring left = sl_patterns[0];
          wstring right = *it4;
          wstring right_pattern = operationToPattern(right); // just the pattern part of the operation
          wstring left_pattern = sl_patterns[0].substr(1, left.length()-1); // all tags are in < >
          s = transducer.insertSingleTransduction(alphabet(alphabet(left.c_str()), alphabet(right.c_str())), k);
          if(patterns.count(alphabet(right.c_str())) < 1)
          {
            RegexpCompiler re;
            re.initialize(&alphabet);
            re.compile(right_pattern);
            Transducer t = re.getTransducer();
           // t.determinize(); // We determinise but not minimise
            patterns[alphabet(right.c_str())] = t;
          }
          if(patterns.count(alphabet(left.c_str())) < 1)
          {
            RegexpCompiler re;
            re.initialize(&alphabet);
            re.compile(left_pattern);
            Transducer t = re.getTransducer();
           // t.determinize(); // Determinise, don't minimise
            patterns[alphabet(left.c_str())] = t;
          }
          //fwprintf(stderr, L"  [%d] sl: %S, tl: %S\n", pos, sl_patterns[0].c_str(), it4->c_str()); 
        }
      }
      else
      {
        int k = s;
        vector<int> reached_states;
        for(vector<wstring>::iterator it4 = rule.sl_context[pos].begin(); it4 != rule.sl_context[pos].end(); it4++)
        {
          wstring left = *it4;
          wstring right = L"<skip(*)>";
          wstring left_pattern = left.substr(1, left.length()-1); // all tags are in < > 
          RegexpCompiler re;
          re.initialize(&alphabet);
          re.compile(left_pattern);
          if(!alphabet.isSymbolDefined(right.c_str()))
          {
            alphabet.includeSymbol(right.c_str());
          }
          s = transducer.insertSingleTransduction(alphabet(alphabet(left.c_str()), alphabet(right.c_str())), k);
          reached_states.push_back(s);
          if(patterns.count(alphabet(left.c_str())) < 1)
          {
            Transducer t = re.getTransducer();
            //t.determinize();
            patterns[alphabet(left.c_str())] = t;
          }
          //fwprintf(stderr, L"%d %d [%d] sl: %S, tl: skip(*)\n", k, s, pos, it4->c_str()); 
        }
        if(reached_states.size() > 1)
        {
          for(vector<int>::iterator x = reached_states.begin(); x != reached_states.end(); x++)
          { 
            int _x = *x;
            //fwprintf(stderr, L"link: %d -> %d\n", _x, s);
            transducer.linkStates(_x, s, 0); 
          }
        }
      }

    }
    wstring id_sym = L"<" + w_id + L">";
    if(!alphabet.isSymbolDefined(id_sym.c_str()))
    {
      alphabet.includeSymbol(id_sym.c_str());
    }
    
    s = transducer.insertSingleTransduction(alphabet(0, alphabet(id_sym.c_str())), s);
    transducer.setFinal(s);
    //wcout << endl;
  }
  transducer.minimize();
  //wcout << transducer.size() << L" " << patterns.size() << endl;
  if(outputGraph)
  {
    fwprintf(stderr, L"\n\n");
    transducer.show(alphabet, stderr);
  }

  for(map<int, Transducer>::iterator it3 = patterns.begin(); it3 != patterns.end(); it3++) 
  {
    wstring sym;
    alphabet.getSymbol(sym, it3->first, false);
    //wcout << it3->first << L" " << it3->second.size() << L" " << sym << endl;
  }
  wcout << endl;

  xmlFreeTextReader(reader);
  xmlCleanupParser();

  return;
}

wstring
LRXCompiler::operationToPattern(wstring op)
{
  // extracts a pattern like season<n>[0-9A-Za-z <>]* from 
  // an operation like select(season<n>[0-9A-Za-z <>]*) 
 
  wstring patron = L"";
  int pc = 0;

  for(wstring::iterator it = op.begin(); it != op.end(); it++)
  {
    if(*it == L'(') 
    { 
      pc++;
      continue;
    }
    if(*it == L')') 
    {
      pc--;
      continue;
    }

    if(pc >= 1)
    {
      patron = patron + *it;
    } 
  }

  return patron;
}

wstring
LRXCompiler::attribsToPattern(wstring lemma, wstring tags)
{

  // turns lemma and tag attributes into a pattern to be compiled into a regex,
  // e.g. "el seu" det.pos.* -> el seu<det><pos>[0-9A-Za-z <>]*

  wstring tl_pattern = L"";

  // the number of full stops, the behaviour is, if it is the 
  // final full stop then put '>' otherwise put '><'
  int fs = 0; 
 
  if(lemma.size() == 0)
  {
    tl_pattern += LRX_COMPILER_ASTERISK;
  }
  else
  {
    tl_pattern = L"";
  }

  for(wstring::iterator it = lemma.begin(); it != lemma.end(); it++) 
  {
    if(*it == L'*')
    {
      tl_pattern += LRX_COMPILER_ASTERISK;
    }
    else
    {
      tl_pattern += *it;
    }
  }

  tl_pattern = tl_pattern + L"<";
 
  if(tags.size() == 0)
  {
    tl_pattern += LRX_COMPILER_ASTERISK;
    return tl_pattern;
  }

/*
  for(wstring::iterator it = tags.begin(); it != tags.end(); it++) 
  {
    if(*it == L'.')
    {
      fs++;
    }
  }
  */
  for(wstring::iterator it = tags.begin(); it != tags.end(); it++) 
  {
    if(*it == L'.')
    {
      //fs--;
/*
      if(fs == 0)
      {
        tl_pattern += L"<";
      }
      else
      {*/
        tl_pattern += L"><";
      /*}*/
    }
    else if(*it == L'*')
    {
      tl_pattern += LRX_COMPILER_ASTERISK;
    }
    else
    {
      tl_pattern += *it;
    }
  }

  return tl_pattern;
}

void
LRXCompiler::procRemove()
{

  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

  //int tipo = xmlTextReaderNodeType(reader);

  rules[current_rule_id].ops++;

  wstring tl_pattern = L"<remove(" + attribsToPattern(lemma, tags) + L")>";
  if(!alphabet.isSymbolDefined(tl_pattern.c_str()))
  {
    alphabet.includeSymbol(tl_pattern.c_str());
  }
  rules[current_rule_id].tl_context[current_rule_len].push_back(tl_pattern);

  //wcout << L"    Remove[" << tipo << "]: " << tl_pattern << endl;
}

void
LRXCompiler::procSelect()
{

  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);
  //int tipo = xmlTextReaderNodeType(reader);

  rules[current_rule_id].ops++;

  wstring tl_pattern = L"<select(" + attribsToPattern(lemma, tags) + L")>";
  if(!alphabet.isSymbolDefined(tl_pattern.c_str()))
  {
    alphabet.includeSymbol(tl_pattern.c_str());
  }
  rules[current_rule_id].tl_context[current_rule_len].push_back(tl_pattern);

  //wcout << L"    Select[" << tipo << "]: " << tl_pattern << endl;
}

void
LRXCompiler::procMatch()
{
  wstring lemma = this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags = this->attrib(LRX_COMPILER_TAGS_ATTR);
  //int tipo = xmlTextReaderNodeType(reader);
  
  wstring sl_pattern = L"";
  if(lemma == L"" && tags == L"")
  {
    sl_pattern = L"<" + LRX_COMPILER_ASTERISK + L">";
  }
  else
  {
    sl_pattern = L"<" + attribsToPattern(lemma, tags) + L">"; 
  }

  if(!alphabet.isSymbolDefined(sl_pattern.c_str()))
  {
    alphabet.includeSymbol(sl_pattern.c_str());
  }
  //rules[current_rule_id].sl_pattern = sl_pattern;
  rules[current_rule_id].sl_context[current_rule_len].push_back(sl_pattern);

  //wcout << L"  Match[" << current_rule_len << L"]: " << sl_pattern << endl;

  if(xmlTextReaderIsEmptyElement(reader))
  {
    //wcout << L"  ~Match[" << tipo << L"]: " << endl;
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
    //tipo = xmlTextReaderNodeType(reader);
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
      break;
    }
    else
    {
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_MATCH_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  }  
}

void
LRXCompiler::procOr()
{

  //current_pattern = L"<(";
  current_rule_len++;
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
      //wcout << L"  " ;
      procMatch();
      //current_pattern = current_pattern + L"|";
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      //current_pattern = current_pattern.substr(0, current_pattern.length()-1) + L")>";
      //if(!alphabet.isSymbolDefined(current_pattern.c_str()))
      //{
      //  alphabet.includeSymbol(current_pattern.c_str());
      //}
      //wcout << L"  Or: " << current_pattern << endl;
      if(current_pattern != L"")
      {
        rules[current_rule_id].sl_context[current_rule_len].push_back(current_pattern);
      }
      current_pattern = L"";
      return;
    }
    else
    {
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_OR_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  }  
}

void
LRXCompiler::procRule()
{
  wstring comment =this->attrib(LRX_COMPILER_C_ATTR);
  current_context_pos = 0;

  rules[current_rule_id].id = current_rule_id;
  rules[current_rule_id].weight = LRX_COMPILER_DEFAULT_WEIGHT;
  rules[current_rule_id].line = xmlTextReaderGetParserLineNumber(reader);
  rules[current_rule_id].ops = 0;
  //int tipo = xmlTextReaderNodeType(reader);

  //wcout << L"Rule " << current_rule_id << L" (line " << rules[current_rule_id].line << L"):" << endl;
 
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
      current_rule_len++;
      procMatch();
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      //wcout << L"  Or:" << endl;
      procOr();
    }
    else if(name == LRX_COMPILER_RULE_ELEM)
    {
      rules[current_rule_id].len = current_rule_len;
      //wcout << L" Len: " << rules[current_rule_id].len << L" "  << endl; 
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
    current_rule_id++;
    current_rule_len = 0;
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
LRXCompiler::write(FILE *fst)
{
  alphabet.write(fst);
  Compression::multibyte_write(patterns.size(), fst);
  for(map<int, Transducer>::iterator it = patterns.begin(); it != patterns.end(); it++) 
  {
    wstring id = itow(it->first);
    //wcout << id << " " << it->second.size() << endl;
    Compression::wstring_write(id, fst);
    it->second.write(fst);
  } 
  Compression::wstring_write(L"main", fst);
  transducer.write(fst);

  for(map<int, LSRule>::iterator it2 = rules.begin(); it2 != rules.end(); it2++) 
  {
    LSRuleRecord record = {  
      it2->second.id, 
      it2->second.len,
      it2->second.ops,
      it2->second.weight 
    };
    fwrite((void *)&record, 1, sizeof(record), fst);
  }

  fwprintf(stderr, L"%d@%d %d\n", transducer.size(), transducer.numberOfTransitions(), transducer.getStateSize(transducer.getInitial()));
  fwprintf(stderr, L"Written %d rules, %d patterns.\n", rules.size(), patterns.size());

  return;
}

