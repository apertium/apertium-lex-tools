#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <list>
#include <set>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/pool.h>
#include <lttoolbox/state.h>
#include <lttoolbox/exception.h>
#include <lttoolbox/trans_exe.h>

using namespace std;

#define PACKAGE_VERSION "0.1.0"
#define MAX_WORDS 300



int readGeneration(FILE *input, FILE *output);
void skipUntil(FILE *input, FILE *output, wint_t const character);
wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);
wchar_t readEscaped(FILE *input);
void streamError();
map< pair<int, wstring>, vector<int> > rule_pos_from_string(int pos, wstring w);



typedef struct SL 
{
  int pos;
  wstring lu;
} SL;

// {0, prova<n><f><sg>: [proof<n><sg>, event<n><sg>, exam<n><sg>, trial<n><sg>, test<n><sg>]}
map< pair<int, wstring>, vector<wstring> > sentence;

typedef struct LSRuleExe
{
  int id;                       // id (e.g. line number) of the rule
  int len;                      // length of the pattern (in LUs)
  double weight;                // an arbitrary rule weight

} LSRuleExe;

map<int, LSRuleExe> rules;

//map<wstring, TransExe> transducers; 
Alphabet alphabet;
State *initial_state;
set<Node *> anfinals;
set<wchar_t> escaped;
bool outOfWord = true;
map<int, Transducer> transducers;


void
streamError()
{
  throw Exception("Error: Malformed input stream.");
}

wchar_t
readEscaped(FILE *input)
{
  if(feof(input))
  {
    streamError();
  }

  wchar_t val = static_cast<wchar_t>(fgetwc_unlocked(input));

  if(feof(input) || escaped.find(val) == escaped.end())
  {
    streamError();
  }

  return val;
}


wstring
readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc_unlocked(input));
    result += c;
    if(c != L'\\')
    {
      continue;
    }
    else
    {
      result += static_cast<wchar_t>(readEscaped(input));
    }
  }

  if(c != delim2)
  {
    streamError();
  }

  return result;
}


void
skipUntil(FILE *input, FILE *output, wint_t const character)
{
  while(true)
  {
    wint_t val = fgetwc_unlocked(input);
    if(feof(input))
    {
      return;
    }

    switch(val)
    {
      case L'\\':
        val = fgetwc_unlocked(input);
        if(feof(input))
        {
          return;
        }
        fputwc_unlocked(L'\\', output);
        fputwc_unlocked(val, output);
        break;

      case L'\0':
        fputwc_unlocked(val, output);
        break;

      default:
        if(val == character)
        {
          return;
        }
        else
        {
          fputwc_unlocked(val, output);
        }
        break;
    }
  }
}


int
readGeneration(FILE *input, FILE *output)
{
  wint_t val = fgetwc_unlocked(input);

  if(feof(input))
  {
    return 0x7fffffff;
  }

  if(outOfWord)
  {
    if(val == L'^')
    {
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    else if(val == L'\\')
    {
      fputwc_unlocked(val, output);
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
      fputwc_unlocked(val,output);
      skipUntil(input, output, L'^');
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    else
    {
      fputwc_unlocked(val, output);
      skipUntil(input, output, L'^');
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    outOfWord = false;
  }

  if(val == L'\\')
  {
    val = fgetwc_unlocked(input);
    return static_cast<int>(val);
  }
  else if(val == L'$')
  {
    outOfWord = true;
    return static_cast<int>(L'$');
  }
  else if(val == L'[')
  {
    fputws_unlocked(readFullBlock(input, L'[', L']').c_str(), output);
    return readGeneration(input, output);
  }
  else
  {
    return static_cast<int>(val);
  }

  return 0x7fffffff;
}



void
readSentence(FILE *in, FILE *ous)
{
  int lu_count = 0;
  pair<int, wstring> cur_sl;  
  vector<wstring> cur_tl;
  State current_state = *initial_state;

  wstring v= L"";

  int val = 0, i = 0;
  bool seenFirst = false;
  wstring sl = L"";
  wstring tl = L"";
  set<wstring> tllu;
  set<wstring> tllu_defaults;

  skipUntil(in, ous, L'^');
  outOfWord = false;

  while((val = readGeneration(in, ous)) != 0x7fffffff)
  {
    switch(val) 
    { 
      case L'^':
        outOfWord = false;
	val = readGeneration(in, ous);
        break;
      case L'/':
        if(!seenFirst) 
        { 
          seenFirst = true;

        } 
        else 
        {
          tllu.insert(tl);
        }
        i++;
        tl = L"";
	val = readGeneration(in, ous);
        if(val != L'$')  
        {
          break;
        } 
      case L'$':
        lu_count++;
        outOfWord = true;
        if(!seenFirst) 
        { 
          seenFirst = true;
        } 
        else 
        {
          tllu.insert(tl);
        }
        // map< pair<int, wstring>, vector<wstring> > sentence;
        cur_sl = make_pair(lu_count, sl);
        if(sentence.find(cur_sl) == sentence.end()) 
        {
          sentence[cur_sl] = cur_tl;
        }

        seenFirst = false;
        //fputws_unlocked(L"^", ous);
        //fputws_unlocked(sl.c_str(), ous);
        for(set<wstring>::const_iterator it = tllu.begin(), j = tllu.end(); it != j; it++)
        {
          if(it != tllu.end())
          {
            //fputws_unlocked(L"/", ous);
          }
          wstring t = *it;
          //fputws_unlocked(t.c_str(), ous);
          sentence[cur_sl].push_back(t);
        }
        //fputws_unlocked(L"$", ous);
        sl = L""; tl = L"";       
        tllu.clear();
        i = 0;
        break;
    }
    if(!seenFirst && !outOfWord) 
    {
      sl.append(1, static_cast<wchar_t>(val));
    }
    else if(!outOfWord)
    { 
      tl.append(1, static_cast<wchar_t>(val));
    }
  }   


  //
  // Collect rules
  //
  //   pos  id   len  operation
  map< int, wstring> operations;  
  int cur_pos = 0;
  for(map< pair<int, wstring>, vector<wstring> >::iterator it = sentence.begin(); 
      it != sentence.end(); it++) 
  {
    pair<int, wstring> sl_pair = it->first;
    vector<wstring> tl_lloc = it->second;

    fwprintf(stderr, L"%d %S: %d\n", sl_pair.first, sl_pair.second.c_str(), tl_lloc.size());
    if(current_state.size() == 0) 
    {
      cur_pos = sl_pair.first;
      current_state = *initial_state;
    }
    current_state.step(sl_pair.second, transducers, alphabet, ous);
    if(current_state.isFinal(anfinals))
    {
      wstring out = current_state.filterFinals(anfinals, alphabet, escaped);
      fwprintf(stderr, L"FINAL: %d %S: %d\n", sl_pair.first, sl_pair.second.c_str(), tl_lloc.size());
      fwprintf(stderr, L"Path: %S\n", out.c_str());
      operations[cur_pos] = out;
    }
  }

  fwprintf(stderr, L"\n");
  int pos = 1;
  for(map< pair<int, wstring>, vector<wstring> >::iterator it = sentence.begin(); 
      it != sentence.end(); it++) 
  {
    pair<int, wstring> sl_pair = it->first;
    vector<wstring> tl_lloc = it->second;
    int j = sl_pair.first;
    vector<wstring> ops;
    for(map< int, wstring>::iterator it2 = operations.begin(); it2 != operations.end(); it2++)
    {
      fwprintf(ous, L"* %d -> %S\n", it2->first, it2->second.c_str());
    
      // pos, op => {rule_id, rule_id, ...}
      map< pair<int, wstring>, vector<int> > rule_pos = rule_pos_from_string(it2->first, it2->second);

      for(map< pair<int, wstring>, vector<int> >::iterator it3 = rule_pos.begin(); it3 != rule_pos.end(); it3++)
      {
        pair<int, wstring> r = it3->first;
        if(r.first == pos) 
        {
          ops.push_back(r.second);
        }
      }

      fwprintf(ous, L"** %d %S ! %d %d ** \n", j, sl_pair.second.c_str(), tl_lloc.size(), ops.size());
      vector<wstring> new_tlloc;
      for(vector<wstring>::iterator it4 = ops.begin(); it4 != ops.end(); it4++) 
      {
        wstring x = *it4;
        // sentence[sl_pair] = tl_lloc
        fwprintf(ous, L"*** x: %S \n", x.c_str());

        for(vector<wstring>::iterator it6 = tl_lloc.begin(); it6 != tl_lloc.end(); it6++) 
        { 
          wstring tl_pattern = L"";
          int parens = 0;
          for(wstring::iterator it7 = x.begin(); it7 != x.end(); it7++)
          { 
            if(*it7 == L'(')
            {
              parens++;
              continue;
            } 
            else if(*it7 == L')')
            {
              parens--;
              continue;
            } 
            
            if(parens > 0) 
            {
              tl_pattern = tl_pattern + *it7;
            }
          }
          if(x.find(L"<skip(") != wstring::npos) 
          {
            fwprintf(stderr, L"SKIP: %S %S %S %S\n", sl_pair.second.c_str(), it6->c_str(), x.c_str(), tl_pattern.c_str());
            new_tlloc = tl_lloc;
            break;

          }
          //RegexpCompiler re;
          //Alphabet alphabet;
          //re.initialize(&alphabet);
          //re.compile(tl_pattern);
          //Transducer tl_pattern_re = re.getTransducer();
          Transducer tl_pattern_re = transducers[alphabet(x)];
          //tl_pattern_re.minimize();
          bool matched = false;
          matched = tl_pattern_re.recognise(*it6, alphabet, stderr);
          if(x.find(L"<select(") != wstring::npos)
          {
            fwprintf(stderr, L"SELECT: %S %S %S %S = %d\n", sl_pair.second.c_str(), it6->c_str(), x.c_str(), tl_pattern.c_str(), matched);
            if(matched)
            {
              new_tlloc.push_back(*it6);
            }
          }
          else if(x.find(L"<remove(") != wstring::npos)
          {
            fwprintf(stderr, L"REMOVE: %S %S %S %S = %d\n", sl_pair.second.c_str(), it6->c_str(), x.c_str(), tl_pattern.c_str(), matched);
            if(!matched)
            {
              new_tlloc.push_back(*it6);
            }
          }
          else
          {
            fwprintf(stderr, L"unsupported operation\n");
          }
        } 
      }
      if(new_tlloc.size() > 0)
      {
        sentence[sl_pair] = new_tlloc;
      }
    }
    pos++;
  }

  for(map< pair<int, wstring>, vector<wstring> >::iterator it = sentence.begin(); 
      it != sentence.end(); it++) 
  {
    pair<int, wstring> sl_pair = it->first;
    vector<wstring> tl_lloc = it->second;
    int j = sl_pair.first;

    fwprintf(ous, L"^%S", sl_pair.second.c_str());
    for(vector<wstring>::iterator it5 = tl_lloc.begin(); it5 != tl_lloc.end(); it5++)
    { 
      if(it5 != tl_lloc.end()) 
      {
        fwprintf(ous, L"/");
      }
      wstring tli = *it5;
      fwprintf(ous, L"%S", tli.c_str());
    }
    fwprintf(ous, L"$ ");
  }
}

map< pair<int, wstring>, vector<int> > 
rule_pos_from_string(int pos, wstring w)
{
  // pos, op => {rule_id, rule_id, ...}
  map< pair<int, wstring>, vector<int> > pos_rule;
 
  // /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><24>

  // [pos+0] = <skip(*)>
  // [pos+1] = <select(season<n>[0-9A-Za-z <>]*)>
  // [pos+2] = <skip(*)>
  // [pos+3] = <skip(*)>

  vector<wstring> matched_rules;

  wstring loc_buf = L"";
  for(wstring::iterator it = w.begin(); it != w.end(); it++)
  {
    if(*it == L'/') 
    { 
      matched_rules.push_back(loc_buf);
      loc_buf = L"";
    }
    loc_buf = loc_buf + *it;
  }
  matched_rules.push_back(loc_buf);
  int pcount = 0;
  wstring temp = L"";
  int acount = 0; // angle bracket (lt/gt) count
  
  for(vector<wstring>::iterator it0 = matched_rules.begin(); it0 != matched_rules.end(); it0++)
  {
    pcount = 0;
    wstring wm = *it0;
    wstring len_buf = L"";
    wstring id_buf = L"";
    for(wstring::iterator it = wm.end(); it != wm.begin(); it--)
    {
      if(*it == L'>')
      { 
        continue;
      }
      if(*it == L'<')
      {
        break;
      }
      id_buf = id_buf + *it;
    }
    reverse(id_buf.begin(), id_buf.end());
  
    wistringstream wstrm2(id_buf);
    int p_id = -1; // The rule id
    wstrm2 >> p_id;

    int p_len = rules[p_id].len;
   
    if(p_len > 0) 
    { 
      fwprintf(stderr, L"len: %d id: %d\n", p_len, p_id);
    }
    for(wstring::iterator it = wm.begin(); it != wm.end(); it++)
    {
      if(*it == L'<') 
      {
        acount++;
      } 
      if(*it == L'>')
      {
        acount--;
        if(acount == 0)
        {
          temp = temp + *it;
          pair<int, wstring> key = make_pair(pos + pcount, temp);
          pos_rule[key].push_back(p_id);
          temp = L"";
          pcount++;
        }
        if(pcount > p_len)
        { 
          break; 
        }
      }
   
      if(acount > 0) 
      {
        temp = temp + *it;
      }
    }
  }
/*
  fwprintf(stdout, L"====\n");
  for(map< pair<int, wstring>, vector<int> >::iterator it2 = pos_rule.begin(); it2 != pos_rule.end(); it2++)
  {
    pair<int, wstring> r = it2->first;
    vector<int> rid = it2->second;

    fwprintf(stdout, L"%d => %S: ", r.first, r.second.c_str());
    for(vector<int>::iterator it3 = rid.begin(); it3 != rid.end(); it3++)
    {
      fwprintf(stdout, L"%d ", *it3);
    }
    fwprintf(stdout, L"\n");
  }
  fwprintf(stdout, L"====\n");
*/
  return pos_rule;

}

int 
main (int argc, char** argv)
{
  Transducer t;
  TransExe te;
  map<int, Transducer> patterns;

  LtLocale::tryToSetLocale();

  escaped.insert(L'$');

  FILE *in = stdin;
  FILE *ous = stdout;

  FILE *fst;
  fst = fopen(argv[1], "r");

  alphabet.read(fst);                 
  //alphabet.show(ous);
  int len = Compression::multibyte_read(fst); 

  fwprintf(stderr, L"%d\n", len);

  while(len > 0)
  { 
    int len2 = Compression::multibyte_read(fst);
    wstring name = L"";
    while(len2 > 0)
    {
      name += static_cast<wchar_t>(Compression::multibyte_read(fst));
      len2--;
    }
    wistringstream wstrm(name);
    int i_name = -655;
    wstrm >> i_name;

    transducers[i_name].read(fst);
    len--;
  }
  fwprintf(stderr, L"Patterns: %d, Alphabet: %d\n", transducers.size(), alphabet.size());

  for(map<int, Transducer>::iterator it = transducers.begin(); it != transducers.end(); it++)
  {
    wstring sym;
    alphabet.getSymbol(sym, it->first, false);
    fwprintf(stderr, L"= %d (%d) =============================\n", it->first, it->second.size());
    fwprintf(stderr, L"  %S\n", sym.c_str());
    it->second.show(alphabet, stderr);
  }

  int len3 = Compression::multibyte_read(fst);
  wstring name = L"";
  while(len3 > 0)
  {
      name += static_cast<wchar_t>(Compression::multibyte_read(fst));
      len3--;
  }
  //wcout << name << endl;
  te.read(fst, alphabet);
  //t.show(alphabet, ous); 

  while(!feof(fst))
  {
    LSRuleExe rec;
    fread(&rec, sizeof(LSRuleExe), 1, fst);
    fwprintf(stderr, L"%d len(%d) weight(%f)\n", rec.id, rec.len, rec.weight);
    rules[rec.id] = rec;
  }

  fclose(fst);

  Pool<vector<int> > *pool = new Pool<vector<int> >(1, vector<int>(50));
  initial_state = new State(pool);
  initial_state->init(te.getInitial());

  anfinals.insert(te.getFinals().begin(), te.getFinals().end());

  //
  // Main loop
  //

  readSentence(in, ous);

  return 0;
}
