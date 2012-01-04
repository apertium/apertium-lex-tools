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

#include <lrx_processor.h>
#include <lrx_config.h>

using namespace std;

wstring const LRXProcessor::LRX_PROCESSOR_S_BOUNDARY = L".<sent>";
unsigned int const LRXProcessor::LRX_PROCESSOR_MAX_S_LENGTH = 300 ;

wstring
LRXProcessor::itow(int i)
{
  // Convert an int to a wstring
  wchar_t buf[50];
  memset(buf, '\0', sizeof(buf));
  swprintf(buf, 50, L"%d", i);
  wstring id(buf);
  return id;
}

int 
LRXProcessor::wtoi(wstring w)
{
  // Convert a wstring to an int
  wistringstream wstrm(w);
  int i_name = -655;
  wstrm >> i_name;

  return i_name;
}

LRXProcessor::LRXProcessor()
{
  pool = new Pool<vector<int> >(4, vector<int>(50));

  initial_state = new State(pool);

  traceMode = false;
  debugMode = false;
  outOfWord = true;
  pos = 0;
  current_line = 1;
}

LRXProcessor::~LRXProcessor()
{
  delete initial_state;
  delete pool;
}

void
LRXProcessor::setTraceMode(bool m)
{
  traceMode = m;
}

void
LRXProcessor::setDebugMode(bool m)
{
  debugMode = m;
}

void
LRXProcessor::load(FILE *in)
{
  // Read in the alphabet 

  alphabet.read(in);

  // Read in the individual patterns

  int len = Compression::multibyte_read(in);

  while(len > 0)
  {
    int len2 = Compression::multibyte_read(in);
    wstring name = L"";
    while(len2 > 0)
    {
      name += static_cast<wchar_t>(Compression::multibyte_read(in));
      len2--;
    }
    int i_name = wtoi(name);
    Transducer t;
    t.read(in);
/*
    map<int, int> finals;
    for(int i = 0; i < t.size(); i++) 
    { 
      if(!t.isFinal(i))
      {
        continue;
      }
      finals[i] = i;
    } 
    MatchExe me(t, finals);
    patterns[i_name] = me;
*/
    wstring sym;
    alphabet.getSymbol(sym, i_name);
    //fwprintf(stderr, L"%d %S %S %d\n", i_name, name.c_str(), sym.c_str(), t.size());
    patterns[i_name] = t;
    len--;
  }

  if(debugMode)
  {
    fwprintf(stderr, L"Patterns: %d, Alphabet: %d\n", patterns.size(), alphabet.size());
  }

  // Now read in the main rule pattern transducer

  int len3 = Compression::multibyte_read(in);

  wstring name = L"";
  while(len3 > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(in));
    len3--;
  }

  // For debugging
  //Transducer v;
  //v.read(in);
  //v.show(alphabet, stderr);

  transducer.read(in, alphabet);

  // skip rule
  rules[0].id = 0;
  rules[0].len = 1;
  rules[0].ops = 0;
  rules[0].weight = 1.0;

  // Now read in the rule records, for lengths/weights
  while(!feof(in))
  {
    LSRuleExe rec;
    fread(&rec, sizeof(LSRuleExe), 1, in);
    if(debugMode) 
    {
      fwprintf(stderr, L"%d len(%d) weight(%f)\n", rec.id, rec.len, rec.weight);
    }
    rules[rec.id] = rec;
  }

  //fclose(in);
 
  return;
}

void 
LRXProcessor::init()
{
  initial_state->init(transducer.getInitial());

  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());
}

vector<int>
LRXProcessor::pathsToRules(wstring const path)
{
  vector<wstring> matched_paths;
  vector<int> matched_rules;
 
  wstring loc_buf = L"";
  for(wstring::const_iterator it = path.begin(); it != path.end(); it++)
  {
    if(*it == L'/') 
    { 
      matched_paths.push_back(loc_buf);
      loc_buf = L"";
    }
    loc_buf = loc_buf + *it;
  }
  if(loc_buf.compare(L"") != 0)
  {
    matched_paths.push_back(loc_buf);
  } 

  for(vector<wstring>::iterator it = matched_paths.begin(); it != matched_paths.end(); it++)
  {
    wstring wm = *it;
    wstring id_buf = L"";
    for(wstring::iterator it2 = wm.end(); it2 != wm.begin(); it2--)
    {
      if(*it2 == L'>')
      { 
        continue;
      }
      if(*it2 == L'<')
      {
        break;
      }
      id_buf = id_buf + *it2;
    }
    if(id_buf.compare(L"") != 0) 
    { 
      reverse(id_buf.begin(), id_buf.end());
      int p_id = wtoi(id_buf);
      matched_rules.push_back(p_id); 
    }
  }
    
  return matched_rules;
}

map< int, pair<int, wstring> >
LRXProcessor::ruleToOpsOptimal(wstring rules, int id, int pos)
{
  vector<wstring> matched_paths;

  if(debugMode)
  {
    fwprintf(stderr, L"ruleToOpsOptimal: %d {%d} %S\n", id, pos, rules.c_str());
  }

  /* Example input:
   *  /<select(liikot<V><[0-9A-Za-zà-ÿ <>@\+]*)><skip(*)><2>/<select(liikot<V><[0-9A-Za-zà-ÿ <>@\+]*)><skip(*)><1>
   */

  wstring loc_buf = L"";
  for(wstring::const_iterator it = rules.begin(); it != rules.end(); it++)
  {
    if(*it == L'/')
    {
      matched_paths.push_back(loc_buf);
      loc_buf = L"";
    }
    loc_buf = loc_buf + *it;
  }
  if(loc_buf.compare(L"") != 0)
  {
    matched_paths.push_back(loc_buf);
  }


  map<int, pair<int, wstring> > ops;
  map<int, wstring> offset_op;
  for(vector<wstring>::iterator it = matched_paths.begin(); it != matched_paths.end(); it++)
  {
    // : /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><5>
    // : /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><1>
    // : /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><skip(*)><6>
    // : /<skip(*)><select(station<n>[0-9A-Za-z <>]*)><skip(*)><10>
    // : /<select(liikot<V><[0-9A-Za-zà-ÿ <>@\+]*)><skip(*)><1>
    wstring m = *it;

    if(m == L"") 
    {
      continue;
    }

    int pcount = 0; // parenthesis count
    int acount = 0; // angle bracket (lt/gt) count
    wstring temp = L"";
    bool skip = false;
    // Watch out here for unbalanced angle brackets
    for(wstring::const_iterator it2 = m.begin(); it2 != m.end(); it2++)
    {
      if(*it2 == L'(') 
      {
        skip = true;
      }
      if(*it2 == L')') 
      {
        skip = false;
      }
      if(skip) 
      {
        temp = temp + *it2;
        continue;
      } 

      if(*it2 == L'<')
      {
        acount++;
      }
      if(*it2 == L'>')
      {
        acount--;
        if(acount == 0)
        {
          temp = temp + *it2;
          offset_op[pos + pcount] = temp;
          temp = L"";
          pcount++;
        }
      }

      if(acount > 0)
      {
        temp = temp + *it2;
      }
    }

    wstring rule_id = offset_op[pos + (pcount - 1)];
    wstring rid = L"<" + itow(id) + L">";
    for(map<int, wstring>::iterator it3 = offset_op.begin(); it3 != offset_op.end(); it3++)
    {
      wstring pattern = L"";

      if(rule_id == rid) 
      {
        wstring part = it3->second;
        wstring type = L"skip";
        bool inPar = false;
        if(part.find(L"<select") != wstring::npos)
        {
          type = L"select";
          for(wstring::iterator it4 = part.begin(); it4 != part.end(); it4++) 
          {
            if(*it4 == L'(') 
            {
              inPar = true;
              continue;
            }  
            else if(*it4 == L')')
            {
              inPar = false;
              continue;
            }
            if(inPar)
            {
              pattern = pattern + *it4;
            }
          }
  
        }
        if(part.find(L"<remove") != wstring::npos)
        {
          type = L"remove";
          for(wstring::iterator it4 = part.begin(); it4 != part.end(); it4++) 
          {
            if(*it4 == L'(') 
            {
              inPar = true;
              continue;
            } 
            else if(*it4 == L')')
            {
              inPar = false;
              continue;
            }
  
            if(inPar)
            {
              pattern = pattern + *it4;
            }
          }
  
        }
        
        if(debugMode)
        {
          fwprintf(stderr, L"*%S offset %d: %S\n", rule_id.c_str(), it3->first, it3->second.c_str());
        }
        //ops[make_pair(it3->first, type)] = pattern;
        //ops[make_pair(it3->first, type)] = part;
        //ops[it3->first] = make_pair(type, part);
        ops[it3->first+1] = make_pair(id, part); 
      } 
    }
  }

  return ops;
}

void 
LRXProcessor::readWord(SItem &w, FILE *input, FILE *output)
{
  int val = 0;
  bool first = true;
  wstring tl = L"";

  val = fgetwc_unlocked(input);
  while(val != L'$' && !feof(input))
  {
    if(val == L'\\')
    {
      if(first)
      {
        w.sl += static_cast<wchar_t>(val);
        val = fgetwc_unlocked(input);
        w.sl += static_cast<wchar_t>(val);
        val = fgetwc_unlocked(input);
        continue;
      }
      else
      { 
        tl += static_cast<wchar_t>(val);
        val = fgetwc_unlocked(input);
        tl += static_cast<wchar_t>(val);
        val = fgetwc_unlocked(input);
        continue;
      }
    }

    if(val == L'/' && first)
    {
      first = false;
      val = fgetwc_unlocked(input);
      continue;
    }
    else if(val == L'/')
    {
      w.tl.push_back(tl);
      tl = L"";
      val = fgetwc_unlocked(input);
      continue;
    }

    if(first)
    {
      w.sl += static_cast<wchar_t>(val); 
    }
    else
    {
      tl += static_cast<wchar_t>(val); 
    }

    val = fgetwc_unlocked(input);
  }
  w.tl.push_back(tl);
  
}

void
LRXProcessor::applyRulesOptimal(map<int, SItem> &sentence, FILE *output)
{ 
  unsigned int lim = sentence.size();

  map<int, pair<int, vector<State> > > covers ;
  pair<int, vector<State> > empty_seq;

  covers[-1] = empty_seq;
  covers[-1].first = 0;

  // A map of pairs of 
  // e.g. 
  //      0,1:0  0.0
  //      0,2:5  1.0
  //      1,3:2  1.0
  //      1,2:0  0.0
  //      2,3:0  0.0
  //      3,4:0  0.0
  //      4,5:0  0.0
  //      5,6:0  0.0
  //      5,6:10 1.0

  vector<State> alive_states_clean ; 
  vector<State> alive_states = alive_states_clean ; 
  alive_states.push_back(*initial_state);

  unsigned int i = 0;
  for(i = 0; i < lim; i++) // For each input sentence word
  {
    SItem w = sentence[i];

    vector<State> new_state;
    pair<int, vector<State> > new_best_cover;
    new_best_cover.first = -numeric_limits<int>::max();

    if(debugMode)
    {
      fwprintf(stderr, L"s[%d]: %S (best_cover: %d)\n", i, w.sl.c_str(), new_best_cover.first); 
    }

    for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++) // For each currently alive state
    {
      State s = *it; 
      if(debugMode)
      {
        fwprintf(stderr, L"  step: %S\n", w.sl.c_str());  
      }
      s.step(w.sl, patterns, alphabet, stderr); // Try and step in the rule transducer using the current word
      if(s.size() > 0) // If the current state has outgoing transitions, add it to the new alive states
      {
        new_state.push_back(s); 
      }
 
      vector<int> found_rules;
      if(s.isFinal(anfinals)) // If this is a final state (regardless of if there is more input), then add the match
      {
        wstring out = s.filterFinals(anfinals, alphabet, escaped_chars);
        if(debugMode)
        {
          fwprintf(stderr, L"%d: %S\n", i, out.c_str());
        } 
        found_rules = pathsToRules(out);

        if(found_rules.size() == 0) 
        {
          if(debugMode)
          { 
            fwprintf(stderr, L"    rule 0: 0\n");
          }
        }
        else 
        {
          for(vector<int>::iterator it2 = found_rules.begin(); it2 != found_rules.end(); it2++)
          { 
            pair<int, vector<State> > newseq = covers[(i - rules[*it2].len)];
            vector<State> reached;
            newseq.first = newseq.first + (rules[*it2].ops * rules[*it2].len);
            if(newseq.first > new_best_cover.first)
            {
              if(debugMode) 
              {
                fwprintf(stderr, L"    BEST COVER %d > %d (rules[%d].ops = %d)\n", newseq.first, new_best_cover.first, *it2, rules[*it2].ops);
              }
              State news(pool);
              news.copy(s);
              reached.push_back(news);
              //newseq.second.push_back(news);
              newseq.second = reached;

              new_best_cover = newseq;
              covers[(i - rules[*it2].len)] = newseq;
            }
            if(debugMode)
            {
              fwprintf(stderr, L"    rule %d: %d (new_best_cover size: %d)\n", *it2, rules[*it2].ops, new_best_cover.second.size());
            }
          }
        }
      }
    }
    alive_states = new_state;
    alive_states.push_back(*initial_state);
  }

  map<int, pair<int, wstring> > pos_ops ; 

  for(map<int, pair<int, vector<State> > >::iterator it = covers.begin(); it != covers.end(); it++) 
  {
    pair<int, vector<State> > best = it->second;
    if(debugMode)
    {
      fwprintf(stderr, L"covers[%d] best (score: %d, size: %d)\n", it->first, best.first, best.second.size());
    }

    for(vector<State>::iterator it2 = best.second.begin(); it2 != best.second.end(); it2++) 
    {
      wstring out = it2->filterFinals(anfinals, alphabet, escaped_chars);
      vector<int> found_rules = pathsToRules(out);
      for(vector<int>::iterator it3 = found_rules.begin(); it3 != found_rules.end(); it3++) 
      {
        map< int, pair<int, wstring> > ops = ruleToOpsOptimal(out, *it3, it->first);
        if(debugMode)
        {
          fwprintf(stderr, L"FR: %d\n", ops.size() );
        }
        pos_ops.insert(ops.begin(), ops.end());
      }
      if(debugMode)
      {
        fwprintf(stderr, L"XX: %d, %S\n", best.first, out.c_str());
      }
    }
  }
  if(debugMode)
  {
    fwprintf(stderr, L"pos_ops: %d\n", pos_ops.size());
  }


  // Apply rules

  for(i = 0; i < lim; i++) // For each input sentence word
  {
    SItem w = sentence[i];
    pair<int, wstring> pos_op = pos_ops[i];
    int current_rule = pos_op.first;
    wstring op = pos_op.second; 

    if(debugMode)
    {
      fwprintf(stderr, L"%d [%d] %S | %S\n", pos_ops.size(), i, w.sl.c_str(), op.c_str());
    }
    wstring op_type = L"";
    if(op.find(L"<select") != wstring::npos) 
    {
      op_type = L"select";
    }
    else if(op.find(L"<remove") != wstring::npos) 
    { 
      op_type = L"remove";
    }
    else
    {
      continue;
    }
    Transducer t = patterns[alphabet(op)];

    if(debugMode)
    {
      fwprintf(stderr, L"  => %d \n", t.size());
    }

    vector<wstring> new_tl;
    for(vector<wstring>::const_iterator it2 = sentence[i].tl.begin(); it2 != sentence[i].tl.end(); it2++)
    {
      bool matched = false;
      wstring tlword = *it2;
      matched = t.recognise(tlword, alphabet, stderr);
      if(debugMode)
      {
        fwprintf(stderr, L"T: %d, %S\n", t.size(), tlword.c_str());
      }

      if(op_type == L"select" && matched)
      {
        new_tl.push_back(tlword);
        if(traceMode)
        {
          fwprintf(stderr, L"%d:SELECT:%d %S\n", current_line, current_rule, tlword.c_str());
        }
      }
      else if(op_type == L"remove" && !matched)
      {
        new_tl.push_back(tlword);
      }
      else if(op_type == L"remove" && matched)
      { 
        if(traceMode)
        {
          fwprintf(stderr, L"%d:REMOVE:%d %S\n", current_line, current_rule, tlword.c_str());
        }
        continue;
      }
      else if(op_type == L"select" && !matched)
      {
        continue;
      }
      else
      {
        new_tl.push_back(tlword);
      }
    }

    if(new_tl.size() > 0)
    {
      sentence[i].tl = new_tl;
    }
  }
  sentence.erase(-1);
}

void
LRXProcessor::process(FILE *input, FILE *output) 
{
  map<int, SItem> sentence;   // pos, item
  bool isEscaped = false;
  sentence.clear();

  int val = fgetwc_unlocked(input);
  while(!feof(input))
  {
    if(val == L'\\')
    {
      isEscaped = true;
      sentence[pos].blank += static_cast<wchar_t>(val);
      val = fgetwc_unlocked(input);
      sentence[pos].blank += static_cast<wchar_t>(val);
      val = fgetwc_unlocked(input);
      isEscaped = false;
    }

    if(val == L'^' && !isEscaped && outOfWord)
    { 
      outOfWord = false;
      readWord(sentence[pos], input, output);
      pos++;
    } 

    if(sentence[pos-1].sl.compare(LRX_PROCESSOR_S_BOUNDARY) == 0 || pos >= LRX_PROCESSOR_MAX_S_LENGTH)
    {
      applyRulesOptimal(sentence, output);
      for(map<int, SItem>::iterator it = sentence.begin(); it != sentence.end(); it++)
      { 
        SItem w = it->second;
        if(debugMode)
        {
          fwprintf(stderr, L"sentence[%d]: %S\n", it->first, w.sl.c_str());
        }
        if(w.sl != L"") 
        { 
          fputws_unlocked(w.blank.c_str(), output);
          w.blank = L"";
          fputws_unlocked(L"^", output);
          fputws_unlocked(w.sl.c_str(), output);
          for(vector<wstring>::iterator it2 = w.tl.begin(); it2 != w.tl.end(); it2++) 
          {
            if(it2 != w.tl.end()) 
            {
              fputws_unlocked(L"/", output);
            }
            fputws_unlocked(it2->c_str(), output);
          }
          fputws_unlocked(L"$", output);
        }
      }
      sentence.clear();
      pos = 0;
    }

    if(outOfWord)
    {
      sentence[pos].blank += static_cast<wchar_t>(val);
    }

    outOfWord = true;

    val = fgetwc_unlocked(input);

    if(val == L'\n')
    { 
      current_line++; 
    }
  }      

  // If there is no .<sent> in the input
  //if(sentence.count(-1) == 0 && sentence.size() > 0)
  if(sentence.size() > 0)
  {
    applyRulesOptimal(sentence, output);
    for(map<int, SItem>::const_iterator it = sentence.begin(); it != sentence.end(); it++)
    { 
      SItem w = it->second;
      if(debugMode)
      {
        fwprintf(stderr, L"(nosent) sentence[%d]: %S\n", it->first, w.sl.c_str());
      }
      if(w.sl != L"") 
      { 
        fputws_unlocked(w.blank.c_str(), output);
        w.blank = L"";
        fputws_unlocked(L"^", output);
        fputws_unlocked(w.sl.c_str(), output);
        for(vector<wstring>::iterator it2 = w.tl.begin(); it2 != w.tl.end(); it2++) 
        {
          if(it2 != w.tl.end()) 
          {
            fputws_unlocked(L"/", output);
          }
          fputws_unlocked(it2->c_str(), output);
        }
        fputws_unlocked(L"$", output);
      }
    } 
  }

  //fwprintf(stderr, L"sentence[%d]: %S\n", pos, sentence[pos].sl.c_str());
  fputws_unlocked(sentence[pos].blank.c_str(), output);
  if(sentence[pos].sl != L"") 
  { 
    fputws_unlocked(L"^", output);
    fputws_unlocked(sentence[pos].sl.c_str(), output);
    for(vector<wstring>::iterator it2 = sentence[pos].tl.begin(); it2 != sentence[pos].tl.end(); it2++) 
    {
      if(it2 != sentence[pos].tl.end()) 
      {
        fputws_unlocked(L"/", output);
      }
      fputws_unlocked(it2->c_str(), output);
    }
    fputws_unlocked(L"$", output);
  }

  return;
}
