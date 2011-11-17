

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

  xmlFreeTextReader(reader);
  xmlCleanupParser();

  // Minimise transducers

  return;
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
  wstring tl_pattern = attribsToPattern(lemma, tags);

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
    sl_pattern = LRX_COMPILER_ASTERISK;
  }
  else
  {
    sl_pattern = attribsToPattern(lemma, tags);
  }

  wcout << L"  " << current_rule_len << L" " << sl_pattern << L":skip(*)" << endl;
}

void
LRXCompiler::procSelect()
{
  wstring lemma =this->attrib(LRX_COMPILER_LEMMA_ATTR);
  wstring tags =this->attrib(LRX_COMPILER_TAGS_ATTR);
  rules[current_rule_id].centre = current_rule_len;

  wstring sl_pattern = attribsToPattern(lemma, tags);

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

  wstring sl_pattern = attribsToPattern(lemma, tags);

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
      current_rule_len++;
      procSkip();
    }
    else if(name == LRX_COMPILER_OR_ELEM)
    {
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
      procSelect();
    }
    else if(name == LRX_COMPILER_REMOVE_ELEM)
    {
      current_rule_len++;
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
LRXCompiler::write(FILE *o)
{
  return;
}


