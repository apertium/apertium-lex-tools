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

using namespace std;

wstring const LRXCompiler::LRX_COMPILER_RULES_ELEM      = L"rules";
wstring const LRXCompiler::LRX_COMPILER_RULE_ELEM       = L"rule";
wstring const LRXCompiler::LRX_COMPILER_SKIP_ELEM       = L"skip";
wstring const LRXCompiler::LRX_COMPILER_SELECT_ELEM     = L"select";
wstring const LRXCompiler::LRX_COMPILER_REMOVE_ELEM     = L"remove";
wstring const LRXCompiler::LRX_COMPILER_ACCEPTION_ELEM  = L"acception";
wstring const LRXCompiler::LRX_COMPILER_OR_ELEM         = L"or";
wstring const LRXCompiler::LRX_COMPILER_LEMMA_ATTR      = L"lemma";
wstring const LRXCompiler::LRX_COMPILER_TAGS_ATTR       = L"tags";
wstring const LRXCompiler::LRX_COMPILER_C_ATTR          = L"c";

wstring const LRXCompiler::LRX_COMPILER_ASTERISK        = L"[0-9A-Za-z <>]*";

double const  LRXCompiler::LRX_COMPILER_DEFAULT_WEIGHT   = 1.0;

LRXCompiler::LRXCompiler()
{
  LtLocale::tryToSetLocale();
  current_rule_id = 0;
  current_rule_len = 0;
  current_context_pos = 0;
}

LRXCompiler::~LRXCompiler()
{
}

wstring
LRXCompiler::itow(int i)
{
  wchar_t buf[50];
  memset(buf, '\0', sizeof(buf));
  swprintf(buf, 50, L"%d", i);
  wstring id(buf);
  return id;
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
    int s = transducer.getInitial();
    wstring w_id = itow(rule.id);
    if(!alphabet.isSymbolDefined(w_id.c_str()))
    {
      alphabet.includeSymbol(w_id.c_str());
    }
    for(map<int, wstring>::iterator it3 = rule.sl_context.begin(); it3 != rule.sl_context.end(); it3++) 
    {
      int pos = it3->first;
      wstring pattern = it3->second;
      if(pos == rule.centre) 
      {
        int k = s;
        for(vector<wstring>::iterator it2 = rule.tl_patterns.begin(); it2 != rule.tl_patterns.end(); it2++) 
        {
          wstring left = rule.sl_pattern;
          wstring right = *it2;
          wstring right_pattern = operationToPattern(*it2); // just the pattern part of the operation
          wstring left_pattern = pattern.substr(1, left.length()-1); // all tags are in < >
          wcerr << rule.id << L" " << pos << L" " << rule.type << L" " << rule.sl_pattern << L":" << right << endl;
          s = transducer.insertSingleTransduction(alphabet(alphabet(left.c_str()), alphabet(right.c_str())), k);
          if(patterns.count(alphabet(right.c_str())) < 1)
          {
            RegexpCompiler re;
            re.initialize(&alphabet);
            re.compile(right_pattern);
            Transducer t = re.getTransducer();
            t.minimize();
            patterns[alphabet(right.c_str())] = t;
          }
          if(patterns.count(alphabet(left.c_str())) < 1)
          {
            RegexpCompiler re;
            re.initialize(&alphabet);
            re.compile(left_pattern);
            Transducer t = re.getTransducer();
            t.minimize();
            patterns[alphabet(left.c_str())] = t;
          }
        }
      }
      else
      {
        wcerr << rule.id << L" " << pos << L" " << pattern << L":skip " << endl;
        wstring left = pattern;
        wstring right = L"<skip(*)>";
        wstring left_pattern = pattern.substr(1, left.length()-1); // all tags are in < > 
        RegexpCompiler re;
        re.initialize(&alphabet);
        re.compile(left_pattern);
        if(!alphabet.isSymbolDefined(right.c_str()))
        {
          alphabet.includeSymbol(right.c_str());
        }
        s = transducer.insertSingleTransduction(alphabet(alphabet(left.c_str()), alphabet(right.c_str())), s);
        if(patterns.count(alphabet(left.c_str())) < 1)
        {
          Transducer t = re.getTransducer();
          t.minimize();
          patterns[alphabet(left.c_str())] = t;
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
    wcout << endl;
  }
  transducer.minimize();
  wcout << transducer.size() << L" " << patterns.size() << endl;
  transducer.show(alphabet, stderr);

  for(map<int, Transducer>::iterator it3 = patterns.begin(); it3 != patterns.end(); it3++) 
  {
    wstring sym;
    alphabet.getSymbol(sym, it3->first, false);
    wcout << it3->first << L" " << it3->second.size() << L" " << sym << endl;
  }

  xmlFreeTextReader(reader);
  xmlCleanupParser();

  // Minimise transducers

  return;
}

wstring
LRXCompiler::operationToPattern(wstring op)
{
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

  wstring tl_pattern = L"";

  // the number of full stops, the behaviour is, if it is the 
  // final full stop then put '>' otherwise put '><'
  int fs = 0; 
 
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

  for(wstring::iterator it = tags.begin(); it != tags.end(); it++) 
  {
    if(*it == L'.')
    {
      fs++;
    }
  }
  
  for(wstring::iterator it = tags.begin(); it != tags.end(); it++) 
  {
    if(*it == L'.')
    {
      fs--;
      if(fs == 0)
      {
        tl_pattern += L">";
      }
      else
      {
        tl_pattern += L"><";
      }
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
LRXCompiler::procAcception()
{
  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

  if(lemma == L"" && tags == L"")
  {
    return;
  }
  wstring tl_pattern = L"<" + rules[current_rule_id].type + L"(" + attribsToPattern(lemma, tags) + L")>";
  if(!alphabet.isSymbolDefined(tl_pattern.c_str()))
  {
    alphabet.includeSymbol(tl_pattern.c_str());
  }
  rules[current_rule_id].tl_patterns.push_back(tl_pattern);


  wcout << L"    Acception: " << tl_pattern << endl;
}


void
LRXCompiler::procSkip()
{
  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);

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
  current_pattern = current_pattern + sl_pattern;
  rules[current_rule_id].sl_context[current_rule_len] = sl_pattern;
  wcout << L"  " << current_rule_len << L" " << sl_pattern << L":skip(*)" << endl;
}

void
LRXCompiler::procSelect()
{
  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);
  rules[current_rule_id].centre = current_rule_len;

  wstring sl_pattern = L"<" + attribsToPattern(lemma, tags) + L">"; 
  if(!alphabet.isSymbolDefined(sl_pattern.c_str()))
  {
    alphabet.includeSymbol(sl_pattern.c_str());
  }
  rules[current_rule_id].sl_pattern = sl_pattern;
  rules[current_rule_id].centre = current_rule_len;
  rules[current_rule_id].sl_context[current_rule_len] = sl_pattern;

  wcout << L"  Select: " << sl_pattern << endl;

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

    if(name == LRX_COMPILER_ACCEPTION_ELEM)
    {
      procAcception();
    }
    else if(name == LRX_COMPILER_SELECT_ELEM)
    {
      return;
    }
    else
    {
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_SELECT_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  }  
}

void
LRXCompiler::procRemove()
{
  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);
  rules[current_rule_id].centre = current_rule_len;

  wstring sl_pattern = L"<" + attribsToPattern(lemma, tags) + L">";
  if(!alphabet.isSymbolDefined(sl_pattern.c_str()))
  {
    alphabet.includeSymbol(sl_pattern.c_str());
  }
  rules[current_rule_id].sl_pattern = sl_pattern;
  rules[current_rule_id].centre = current_rule_len;
  rules[current_rule_id].sl_context[current_rule_len] = sl_pattern;

  wcout << L"  Remove: " << sl_pattern << endl;

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

    if(name == LRX_COMPILER_ACCEPTION_ELEM)
    {
      procAcception();
    }
    else if(name == LRX_COMPILER_REMOVE_ELEM)
    {
      return;
    }
    else
    {
      wcerr << L"Error (" << xmlTextReaderGetParserLineNumber(reader);
      wcerr << L"): Invalid inclusion of '<" << name << L">' into '<" << LRX_COMPILER_REMOVE_ELEM;
      wcerr << L">'." << endl; 
      exit(EXIT_FAILURE);
    }
  } 
}

void
LRXCompiler::procOr()
{

  current_pattern = L"<(";
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

    if(name == LRX_COMPILER_SKIP_ELEM)
    {
      wcout << L"  " ;
      procSkip();
      current_pattern = current_pattern + L"|";
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      current_pattern = current_pattern.substr(0, current_pattern.length()-1) + L")>";
      if(!alphabet.isSymbolDefined(current_pattern.c_str()))
      {
        alphabet.includeSymbol(current_pattern.c_str());
      }
      wcout << L"  Or: " << current_pattern << endl;
      rules[current_rule_id].sl_context[current_rule_len] = current_pattern;
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

  wcout << L"Rule " << current_rule_id << L":" << endl;
  rules[current_rule_id].id = current_rule_id;
  rules[current_rule_id].weight = LRX_COMPILER_DEFAULT_WEIGHT;
  
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
    
    if(name == LRX_COMPILER_SKIP_ELEM)
    {      
      current_rule_len++;
      procSkip();
    }
    else if(name == LRX_COMPILER_SELECT_ELEM)
    {
      current_rule_len++;
      rules[current_rule_id].type = L"select";
      procSelect();
    }
    else if(name == LRX_COMPILER_REMOVE_ELEM)
    {
      current_rule_len++;
      rules[current_rule_id].type = L"remove";
      procRemove();
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
      wcout << L"  Or:" << endl;
      procOr();
    }
    else if(name == LRX_COMPILER_RULE_ELEM)
    {
      rules[current_rule_id].len = current_rule_len;
      wcout << L" Len: " << rules[current_rule_id].len << L" " << rules[current_rule_id].centre << endl; 
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
    wcout << id << " " << it->second.size() << endl;
    Compression::wstring_write(id, fst);
    it->second.write(fst);
  } 
  Compression::wstring_write(L"main", fst);
  transducer.write(fst);

  for(map<int, LSRule>::iterator it2 = rules.begin(); it2 != rules.end(); it2++) 
  {
    LSRuleRecord record = {  it2->second.id, it2->second.len, it2->second.weight };
    fwrite((void *)&record, 1, sizeof(record), fst);
  }

  return;
}

