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
  current_state = new State(pool);

  traceMode = false;
  pos = 0;
}

LRXProcessor::~LRXProcessor()
{
  delete current_state;
  delete initial_state;
  delete pool;
}

void
LRXProcessor::setTraceMode(bool m)
{
  traceMode = m;
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

  //fwprintf(stderr, L"Patterns: %d, Alphabet: %d\n", patterns.size(), alphabet.size());

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
    //fwprintf(stderr, L"%d len(%d) weight(%f)\n", rec.id, rec.len, rec.weight);
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

void 
LRXProcessor::applyRules(map<int, SItem> &sentence, FILE *output)
{
  // A map containing spans in the sentence and a list of rules which match
  // these spans
  map< pair<int, int>, vector<int> > rule_spans; 
  map< pair<int, int>, wstring > pos_rules; 

  // List of current states in the rule transducer
  vector<State> current_states; 

  current_states.push_back(*initial_state);

  unsigned int j = 0; // current range start
  unsigned int k = 1; // current range end

  // The default rule is to just skip
  vector<int> skip;
  skip.push_back(0);

  unsigned int lim = sentence.size();
  // Read all of the possible rule applications into a chart
  // and also populate with default 'skip' transitions
  for(unsigned int i = 0; i < lim; i++)
  {
    SItem s = sentence[i];
    //fwprintf(stderr, L"%d | %d [%d:%d] %S(%d) (C: %d)\n", sentence.size(), i, j, k, s.sl.c_str(), s.tl.size(), current_states.size());

    vector<int> rules; // Each time we advance a word in the sentence we reinitialise the list of current matched rules
    State is = *initial_state;
    for(k = j; k < lim; k++) // From the current position in the sentence until the end
    {
      if(is.size() == 0) // FIXME: Should this go after isFinal?
      { 
        break;
      }
      if(is.isFinal(anfinals)) // If this is a final state (regardless of if there is more input), then add the match
      {
        wstring out = is.filterFinals(anfinals, alphabet, escaped_chars);
        //fwprintf(stderr, L"%d->%d : %S\n", j, k, out.c_str());
 
        pair<int, int> span = make_pair(j, k);
        rule_spans[span] = pathsToRules(out);
        pos_rules[span] = out; 
      }
      is.step(sentence[k].sl, patterns, alphabet, stderr);
    }
    k = j; 

    if(i != j) // Add the default 'skip' rule
    {
      pair<int, int> default_span = make_pair(j, i);
      if(rule_spans.find(default_span) == rule_spans.end()) 
      {
        rule_spans[default_span] = skip;
      } 
      else
      {
        rule_spans[default_span].push_back(0);
      }
    }

    j = i;
  }

  int p = 0;

  // Print out the transducer of matched rules
  for(map< pair<int, int>, vector<int> >::iterator it = rule_spans.begin(); it != rule_spans.end(); it++) 
  {
    pair<int, int> span = it->first;
    vector<int> r = it->second;
    for(vector<int>::iterator it2 = r.begin(); it2 != r.end(); it2++)
    {
      fwprintf(stderr, L"%d\t%d\t%d\t%d\n", span.first, span.second, *it2, *it2);
    }
    p = it->first.second;
  }
  fwprintf(stderr, L"%d\n", p); // Final state
 
  // Find the optimal path
  map< pair <int, int>, int > path = bestPath(rule_spans, lim);

/*
  // print out paths + scores
  fwprintf(stderr, L"\n");
  for(map< pair<int, int>, int>::iterator it = path.begin(); it != path.end(); it++)
  {
    pair <int, int> transition = it->first;
    int rule = it->second;
    fwprintf(stderr, L"%d %d *%d *%d\n", transition.first, transition.second, rule, rule);
  }
*/

  // Here is where we apply the rules that best cover the sentence to the sentence
  for(unsigned int i = 0; i < lim; i++)
  { 
    SItem s = sentence[i];
    fwprintf(stderr, L"%d %S(%d)\n", i, s.sl.c_str(), s.tl.size());
    for(unsigned int j = i; j < lim; j++)
    {
      pair<int, int> p = make_pair(i, j);
      if(rule_spans.find(p) != rule_spans.end())
      {
        int rule = path[p];
        if(rule > 0) 
        {
          //int offset = 0;
          map< pair<int, wstring>, wstring> ops = ruleToOps(pos_rules[p], rule, i);
 
          for(map< pair<int, wstring>, wstring>::iterator it = ops.begin(); it != ops.end(); it++)
          {
            pair<int, wstring> oftype = it->first;
            wstring matcher = it->second ;
            fwprintf(stderr, L"-> rule %d: %d %d %f\n", rule, rules[rule].id, rules[rule].len, rules[rule].weight);
            fwprintf(stderr, L": %S\n", pos_rules[p].c_str());
            fwprintf(stderr, L"%d : [%d] %d | %S | %S \n", alphabet(matcher), rule, oftype.first, oftype.second.c_str(), matcher.c_str());
            if(oftype.second == L"skip")
            {  
              continue; 
            }
            Transducer t = patterns[alphabet(matcher)]; 
            vector<wstring> new_tl;
            for(vector<wstring>::const_iterator it2 = sentence[oftype.first].tl.begin(); it2 != sentence[oftype.first].tl.end(); it2++)
            {
              bool matched = false;
              wstring tlword = *it2;
              matched = t.recognise(tlword, alphabet, stderr);
              if(oftype.second == L"select" && matched)
              {
                new_tl.push_back(tlword);
                if(traceMode)
                {
                  fwprintf(stderr, L"SELECT:%d %S\n", rule, tlword.c_str());
                }
              } 
              else if(oftype.second == L"remove" && !matched)
              { 
                new_tl.push_back(tlword);
                if(traceMode)
                {
                  fwprintf(stderr, L"SELECT:%d %S\n", rule, tlword.c_str());
                }
              }
              else if(oftype.second == L"remove" && matched)
              { 
                if(traceMode)
                {
                  fwprintf(stderr, L"REMOVE:%d %S\n", rule, tlword.c_str());
                }
                continue;
              }
              else if(oftype.second == L"select" && !matched)
              {
                if(traceMode)
                {
                  fwprintf(stderr, L"REMOVE:%d %S\n", rule, tlword.c_str());
                }
                continue;
              }
              else
              {
                new_tl.push_back(tlword);
                fwprintf(stderr, L"%d : %d COPY %S\n", t.size(), matched, tlword.c_str());
              }
            }
             
            if(new_tl.size() > 0)
            {
              sentence[oftype.first].tl = new_tl;
            }
          }
        } 
      }
    }
  }
}

map< pair<int, wstring>, wstring> 
LRXProcessor::ruleToOps(wstring rules, int id, int pos)
{
  vector<wstring> matched_paths;

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

  map< pair<int, wstring>, wstring> ops;
  map<int, wstring> offset_op;
  for(vector<wstring>::iterator it = matched_paths.begin(); it != matched_paths.end(); it++)
  {
    // : /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><5>
    // : /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><1>
    // : /<select(season<n>[0-9A-Za-z <>]*)><skip(*)><skip(*)><6>
    // : /<skip(*)><select(station<n>[0-9A-Za-z <>]*)><skip(*)><10>

    wstring m = *it;
    int pcount = 0; // parenthesis count
    int acount = 0; // angle bracket (lt/gt) count
    wstring temp = L"";

    for(wstring::const_iterator it2 = m.begin(); it2 != m.end(); it2++)
    {
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
        
        //fwprintf(stderr, L"*%S offset %d: %S\n", rule_id.c_str(), it3->first, it3->second.c_str());
        //ops[make_pair(it3->first, type)] = pattern;
        ops[make_pair(it3->first, type)] = part;
      } 
    }
  }

  return ops;
}


map< pair<int, int>, int > // This function needs to be optimised
LRXProcessor::bestPath(map< pair<int, int>, vector<int> > &rule_spans, unsigned int slen)
{
  
  map< pair<int, int>, int > path;
  
  map<wstring, int> scores;
  map<wstring, unsigned int> path_last;
  scores[L""] = 0; 

  fwprintf(stderr, L"slen: %d\n\n", slen);
  
  for(unsigned int i = 0; i < slen; i++) 
  {
    map<wstring, int> new_scores;
    for(unsigned int j = i; j < slen; j++)
    {
      pair<int, int> p = make_pair(i, j);
      for(vector<int>::const_iterator it = rule_spans[p].begin(); it != rule_spans[p].end(); it++)
      {
        int current_rule = *it;
        for(map<wstring, int>::const_iterator it3 = scores.begin(); it3 != scores.end(); it3++)
        {
          wstring current_path = it3->first;
          int score = it3->second;
          /*if(current_rule == 0)
          { 
            scores[current_path]--;
          }*/
          if(rule_spans.find(p) != rule_spans.end())
          {
            wstring transition = itow(i) + L">" + itow(j) + L":" + itow(current_rule);;
            wstring new_path = L"";
            if(current_path != L"")
            {
              new_path = current_path + L" " + transition;
            }
            else
            { 
              new_path = transition;
            }
  
            if(i < path_last[current_path])
            {
              //fwprintf(stderr, L"SKIP: \n");
              //fwprintf(stderr, L"  current_path[%S]\n", current_path.c_str());
              //fwprintf(stderr, L"  new_path[%S]\n", new_path.c_str()); 
              //fwprintf(stderr, L"  last[%S] = %d\n", current_path.c_str(), path_last[current_path]); 
              new_scores[current_path] = score;
            }
            else
            {
              new_scores[new_path] = scores[current_path] + rules[current_rule].ops;
              //fwprintf(stderr, L"+ %d new_path[%S] last[%S] = %d\n", it3->second, new_path.c_str(), current_path.c_str(), path_last[current_path]);
              path_last[new_path] = j;
            }
          }
        }
      }
    }
    if(new_scores.size() > 0)
    {
      scores = new_scores;
    }
  }
  //fwprintf(stderr, L"\n\n", scores.size());
  
  double max = 1.0 / static_cast<double>(slen);
  wstring current_max = L"";
  for(map<wstring, int>::iterator it = scores.begin(); it != scores.end(); it++)
  {
    double score = static_cast<double>(it->second) / static_cast<double>(slen);

    if(it->second == 0) 
    {
      score = -1.0;
    }

    if(score > max)
    {
      max = score;
      current_max = it->first;
    }
    fwprintf(stderr, L"max: %f cur: %f | %d path[%S]\n", max, score, it->second, it->first.c_str());
  }
  //fwprintf(stderr, L"max: %S\n", current_max.c_str());

  wstring t = L"";  
  int i = 0;
  int j = 0;
  int rule = 0;
  for(wstring::iterator it = current_max.begin(); it != current_max.end(); it++)
  {
    switch(*it)
    {
      case L' ':
        rule = wtoi(t);
        fwprintf(stderr, L"%d:%d %d\n", i, j, rule);
        path[make_pair(i, j)] = rule;
        i = 0; 
        j = 0; 
        t = L"";
        break;
      case L'>':
        i = wtoi(t);
        t = L"";
        break;
      case L':':
        j = wtoi(t);
        t = L"";
        break;
      default:
        t = t + *it;
    }
  }
  rule = wtoi(t);
  fwprintf(stderr, L"%d:%d %d\n", i, j, rule);
  path[make_pair(i, j)] = rule;

  return path; 
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
      applyRules(sentence, output);
      for(map<int, SItem>::iterator it = sentence.begin(); it != sentence.end(); it++)
      { 
        SItem w = it->second;
        //fwprintf(stderr, L"sentence[%d]: %S\n", it->first, w.sl.c_str());
        fputws_unlocked(w.blank.c_str(), output);
        if(w.sl != L"") 
        { 
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
