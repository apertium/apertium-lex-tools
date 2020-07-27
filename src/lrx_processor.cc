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

#include <lrx_processor.h>
#include <cstdint>
using namespace std;

wstring const LRXProcessor::LRX_PROCESSOR_TAG_SELECT     = L"<select>";
wstring const LRXProcessor::LRX_PROCESSOR_TAG_REMOVE     = L"<remove>";
wstring const LRXProcessor::LRX_PROCESSOR_TAG_SKIP       = L"<skip>";

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


LRXProcessor::LRXProcessor()
{

  initial_state = new State();

  lineno = 1; // Used for rule tracing
  pos = 0;

  traceMode = false;
  debugMode = false;
  outOfWord = true;
  nullFlush = false;
}

LRXProcessor::~LRXProcessor()
{
  delete initial_state;
}

void
LRXProcessor::setTraceMode(bool m)
{
  traceMode = m;
}

void
LRXProcessor::setNullFlush(bool m)
{
  nullFlush = m;
}

void
LRXProcessor::setDebugMode(bool m)
{
  debugMode = m;
}

void
LRXProcessor::load(FILE *in)
{
  alphabet.read(in);

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
    recognisers[name].read(in, alphabet);
    if(debugMode)
    {
      fwprintf(stderr, L"Recogniser: %S, [finals: %d]\n", name.c_str(), recognisers[name].getFinals().size());
    }
    len--;
  }

  if(debugMode)
  {
    fwprintf(stderr, L"recognisers: %d\n", recognisers.size());
  }

  int len3 = Compression::multibyte_read(in);

  wstring name = L"";
  while(len3 > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(in));
    len3--;
  }

  transducer.read(in, alphabet);

  // Now read in weights
  struct weight {
        int32_t id;
        char _pad[4]{};
        double pisu;
  };

  weight record;
  while(fread(&record, sizeof(weight), 1, in))
  {
    wstring sid = L"<" + itow(record.id) + L">";
    weights[sid] = record.pisu;

    if(debugMode)
    {
      //fwprintf(stderr, L"%S %d weight(%.4f)\n", sid.c_str(), record.id, record.pisu);
    }
  }

  return;
}

void
LRXProcessor::init()
{
  initial_state->init(transducer.getInitial());

  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());

  escaped_chars.insert(L'[');
  escaped_chars.insert(L']');
  escaped_chars.insert(L'{');
  escaped_chars.insert(L'}');
  escaped_chars.insert(L'^');
  escaped_chars.insert(L'$');
  escaped_chars.insert(L'/');
  escaped_chars.insert(L'\\');
  escaped_chars.insert(L'@');
  escaped_chars.insert(L'<');
  escaped_chars.insert(L'>');

}

wstring
LRXProcessor::readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc_unlocked(input));
    result += c;
  }

  return result;
}

bool
LRXProcessor::recognisePattern(const wstring lu, const wstring op)
{
  if(recognisers.count(op) < 1)
  {
    fwprintf(stderr, L"WARNING: Recogniser not found for key %S, skipping... [LU: %S]\n", op.c_str(), lu.c_str());
    return false;
  }

  State *first_state = new State();
  first_state->init(recognisers[op].getInitial());
  State cur = *first_state;

  map<Node *, double> end_states;
  end_states.insert(recognisers[op].getFinals().begin(), recognisers[op].getFinals().end());

  bool readingTag = false;
  wstring tag = L"";
  int val = 0;
  for(auto& it : lu)
  {
/*
    if(debugMode)
    {
      fwprintf(stderr, L"alive: %d\n", cur.size());
    }
*/
    if(cur.size() < 1)  // I think that any time we have 0 alive states,
                        // we can say that the string is unrecognised
    {
      return false;
    }
    if(it == L'<')
    {
      tag = L"";
      readingTag = true;
      tag = tag + it;
      continue;
    }
    if(it == L'>')
    {
      tag = tag + it;
      val = static_cast<int>(alphabet(tag));
      if(val == 0)
      {
        val = static_cast<int>(alphabet(L"<ANY_TAG>"));
      }
/*
      if(debugMode)
      {
        fwprintf(stderr, L":: tag %S: %d\n", tag.c_str(), val);
        fwprintf(stderr, L"  step: %S\n", tag.c_str());
      }
*/
      cur.step(val, alphabet(L"<ANY_TAG>"));
      readingTag = false;
      continue;
    }
    if(readingTag)
    {
      tag = tag + it;
    }
    else
    {
      // We're not in a tag, so were just reading characters
      int val = static_cast<int>(it);
/*
      if(debugMode)
      {
        fwprintf(stderr, L"  step: %C\n", val);
      }
*/
      //cur.step(val, a(L"<ANY_CHAR>"));
      //cur.step(val);
      set<int> alts;
      if(!iswupper(val))
      {
        alts.insert(alphabet(L"<ANY_CHAR>"));
        alts.insert(alphabet(L"<ANY_LOWER>"));
      }
      else
      {
        alts.insert(alphabet(L"<ANY_CHAR>"));
        alts.insert(alphabet(L"<ANY_UPPER>"));
        alts.insert(towlower(val));
      }
      cur.step(val, alts);

    }
  }

/*
  if(debugMode)
  {
    fwprintf(stderr, L">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  }
*/
  if(cur.isFinal(end_states))
  {
    return true;
  }

  return false;
}

/*
void
LRXProcessor::processFlush(FILE *output,
                           map<int, wstring > &sl,
                           map<int, vector<wstring> > &tl,
                           map<int, wstring > &blanks,
                           map<int, pair<double, vector<State> > > &covers,
                           pair<double, vector<State> > &empty_seq,
                           map<pair<int, int>, vector<State> > &spans,
                           int last_final)
{
  if(debugMode)
  {
    fwprintf(stderr, L"FLUSH:\n");
  }

  map<int, pair<double, vector<State> > >::iterator it;
  map<int, pair<wstring, wstring> > operations;

  for(it = covers.begin(); it != covers.end(); it++)
  {
    pair<double, vector<State> > best = it->second;
    if(debugMode)
    {
      fwprintf(stderr, L"===================================================\n");
      fwprintf(stderr, L"[%d][%d] covers[%d] best (score: %d, size: %d)\n", pos, last_final, it->first, best.first, best.second.size());
    }

    // return M[i-1]
    if(it->first == last_final)
    {
      vector<State>::iterator it2;
      for(it2 = best.second.begin(); it2 != best.second.end(); it2++)
      {
        if(debugMode)
        {
          wstring out = it2->filterFinals(anfinals, alphabet, escaped_chars);
          fwprintf(stderr, L"!!!    filter_finals: %S\n", out.c_str());
        }
        set<pair<wstring, vector<wstring> > > outpaths;
        outpaths = it2->filterFinalsLRX(anfinals, alphabet, escaped_chars, false, false, 0);

        int j = 1;
        set<pair<wstring, vector<wstring> > >::iterator it3;
        for(it3 = outpaths.begin(); it3 != outpaths.end(); it3++)
        {
          wstring id = it3->first;
          vector<wstring> ops = it3->second;
          vector<wstring>::iterator op;
          for(op = ops.begin(); op != ops.end(); op++)
          {
            if(*op != LRX_PROCESSOR_TAG_SKIP)
            {
              int starting_point = -1;
              map<pair<int, int>, vector<State> >::iterator ix;
              for(ix = spans.begin(); ix != spans.end(); ix++)
              {
                vector<State>::iterator iy;
                for(iy = ix->second.begin(); iy != ix->second.end(); iy++)
                {
                  set<pair<wstring, vector<wstring> > > y;
                  y = iy->filterFinalsLRX(anfinals, alphabet, escaped_chars, false, false, 0);
                  if(y == outpaths)
                  {
                    starting_point = ix->first.first;
                  }
                }
              }
              if(debugMode)
              {
                fwprintf(stderr, L"=> APPLY [pos: %d, dep: %d, j: %d, start: %d, len: %d]: %S // %S\n", pos, starting_point, j, starting_point+j, ops.size(), id.c_str(), op->c_str());
              }
              operations[starting_point+j].first = id;
              operations[starting_point+j].second = *op;
            }
            j++;
          }
        }
        if(debugMode)
        {
          fwprintf(stderr, L"[best: %d, outpaths: %d]\n", best.first, outpaths.size());
        }
      }
    }
  }

  covers.clear();
  covers[-1] = empty_seq;
  covers[-1].first = 0;

  // Here we actually apply the rules that we've matched

  unsigned int spos = 0;
  for(spos = 0; spos <= pos; spos++)
  {
    if(sl[spos] == L"")
    {
      continue;
    }
    wstring  op = operations[spos].second;
    wstring  tipus = L"";
    if(op.find(LRX_PROCESSOR_TAG_SELECT) != wstring::npos)
    {
      tipus = LRX_PROCESSOR_TAG_SELECT;
    }
    if(op.find(LRX_PROCESSOR_TAG_REMOVE) != wstring::npos)
    {
      tipus = LRX_PROCESSOR_TAG_REMOVE;
    }
    if(debugMode)
    {
      fwprintf(stderr, L"#APPL%S. %S\n", tipus.c_str(), op.c_str());
    }

    fwprintf(output, L"%S^%S/", blanks[spos].c_str(), sl[spos].c_str());

    vector<wstring>::iterator ti;
    vector<wstring>::iterator penum = tl[spos].end(); penum--;

    if(tipus == LRX_PROCESSOR_TAG_SELECT && tl[spos].size() > 1)
    {
      bool matched = true;
      bool selected = false;
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        matched = recognisePattern(*ti, op);
        if(matched)
        {
          if(traceMode || debugMode)
          {
            fwprintf(stderr, L"%d:SELECT%S:%S:%S\n", lineno, operations[spos].first.c_str(), sl[spos].c_str(), op.c_str());
          }
          fwprintf(output, L"%S", ti->c_str());
          selected = true;
          break;
        }
      }
      if(!selected)
      {
        for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
        {
          fwprintf(output, L"%S", ti->c_str());
          if(ti != penum)
          {
            fwprintf(output, L"/");
          }
        }
      }
    }
    else if(tipus == LRX_PROCESSOR_TAG_REMOVE && tl[spos].size() > 1)
    {
      bool matched = true;
      vector<wstring> new_tl;  // The new list of TL translations
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        matched = recognisePattern(*ti, op);
        if(matched)
        {
          if(traceMode || debugMode)
          {
            fwprintf(stderr, L"%d:REMOVE%S:%S:%S\n", lineno, operations[spos].first.c_str(), sl[spos].c_str(), op.c_str());
          }
          continue;
        }
        new_tl.push_back(*ti);
      }
      vector<wstring>::iterator nti;
      vector<wstring>::iterator npenum = new_tl.end(); npenum--;
      for(nti = new_tl.begin(); nti != new_tl.end(); nti++)
      {
        fwprintf(output, L"%S", nti->c_str());
        if(nti != npenum)
        {
          fwprintf(output, L"/");
        }
      }
      new_tl.clear();
    }
    else
    {
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        fwprintf(output, L"%S", ti->c_str());
        if(ti != penum)
        {
          fwprintf(output, L"/");
        }
      }
    }
    fwprintf(output, L"$");
    if(debugMode)
    {
      fwprintf(output, L"%d", spos);
    }
  }
}
*/

/*
void
LRXProcessor::process(FILE *input, FILE *output)
{
  bool isEscaped = false;

  map<int, wstring > sl; // map of SL words
  map<int, vector<wstring> > tl; // map of vectors of TL translations
  map<int, wstring > blanks; // map of the superblanks

  map<int, pair<double, vector<State> > > covers ;
  pair<double, vector<State> > empty_seq;
  map<pair<int, int>, vector<State> > spans ;

  covers[-1] = empty_seq;
  covers[-1].first = 1.0;

  vector<State> alive_states_clean ;
  vector<State> alive_states = alive_states_clean ;
  alive_states.push_back(*initial_state);
  vector<State> new_states;

  int last_final = -1; // check what we actually use this for

  while(!feof(input))
  {
    int val = fgetwc_unlocked(input);

    if(nullFlush && val == L'\0')
    {
      processFlush(output, sl, tl, blanks, covers, empty_seq, spans, last_final);
      fwprintf(output, L"%S", blanks[pos].c_str());
      pos = 0;
      last_final = 0;
      tl.clear();
      sl.clear();
      blanks.clear();
      spans.clear();

      fputwc_unlocked(val, output);
      fflush(output);
      continue;
    }

    // We're starting to read a new lexical form
    if(val == L'^' && !isEscaped && outOfWord)
    {
      outOfWord = false;
      continue;
    }

    // We've seen the surface form
    if(val == L'/' && !isEscaped && !outOfWord)
    {
      // Read in target equivalences
      wstring trad = L"";
      val = fgetwc_unlocked(input);
      while(val != L'$')
      {
        if(val != L'$')
        {
          trad += static_cast<wchar_t>(val);
        }
        if(val == L'/')
        {
          tl[pos].push_back(trad.substr(0, trad.length()-1));
          trad = L"";
        }
        val = fgetwc_unlocked(input);
      }
      tl[pos].push_back(trad);

      if(debugMode)
      {
        for(vector<wstring>::iterator it = tl[pos].begin(); it != tl[pos].end(); it++)
        {
          fwprintf(stderr, L"trad[%d]: %S\n", pos, it->c_str());
        }
      }
    }

    // We've finished reading a lexical form
    if((feof(input) || val == L'$') && !isEscaped && !outOfWord)
    {
      if(debugMode)
      {
        fwprintf(stderr, L"[POS] %d: [sl %d ; tl %d ; bl %d]\n", pos, sl[pos].size(), tl[pos].size(), blanks[pos].size());
      }

      new_states.clear(); // alive_states_new
      pair<double, vector<State> > new_best_cover;
      new_best_cover.first = -numeric_limits<int>::max();

      vector<int> matched_rules;

      // \forall s \in A
      for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
      {
        State s = *it;
        // \IF \exists c \in Q : \delta(s, sent[i]) = c
        s.step(alphabet(L"<$>"));

        // A \gets A \cup {c}
        if(s.size() > 0) // If the current state has outgoing transitions,
                         // add it to the new alive states
        {
          new_states.push_back(s);
        }
        s.step(alphabet(L"<$>"));

        // \IF c \in F
        if(s.isFinal(anfinals))
        {
          // We've reached a final state, so we need to evaluate the rule we've matched
          if(debugMode)
          {
            wstring out = s.filterFinals(anfinals, alphabet, escaped_chars);
            fwprintf(stderr, L"    filter_finals: %S\n", out.c_str());
          }

          set<pair<wstring, vector<wstring> > > outpaths;
          outpaths = s.filterFinalsLRX(anfinals, alphabet, escaped_chars, false, false, 0);

          set<pair<wstring, vector<wstring> > >::iterator it;
          for(it = outpaths.begin(); it != outpaths.end(); it++)
          {
            vector<State> reached;

            vector<wstring> path = (*it).second;
            wstring id = (*it).first;

            if(debugMode)
            {
              fwprintf(stderr, L"id:      %S:\n", id.c_str());
              for(vector<wstring>::iterator it2 = path.begin(); it2 != path.end(); it2++)
              {
                fwprintf(stderr, L"op:        %S\n", it2->c_str());
              }
              fwprintf(stderr, L"#SPAN[%d, %d]\n", (pos-path.size()), pos);
            }

            spans[make_pair((pos-path.size()), pos)].push_back(s);

            // M[i-ChunkLength(c)]
            pair<double, vector<State> > newseq = covers[(pos - path.size())];
            newseq.first = newseq.first + path.size() ;

            if(newseq.first > new_best_cover.first)
            {
              State new_state;
              new_state = s;
              reached.push_back(new_state);
              map<int, pair<double, vector<State> > >::iterator k;
              for(k = covers.begin(); k != covers.end(); k++)
              {
                vector<State>::iterator l;
                pair<double, vector<State> > p = k->second;
                for(l = p.second.begin(); l != p.second.end(); l++)
                {
                  if(debugMode)
                  {
                    fwprintf(stderr, L"= [cov: %d][len: %d][pos: %d][pat: %d] INCLUDE FINALS?\n", k->first, p.first, pos, path.size());
                  }
                  if(k->first <= (pos - path.size()))
                  {
                    if(debugMode)
                    {
                      wstring out2 = l->filterFinals(anfinals, alphabet, escaped_chars);
                      fwprintf(stderr, L"    == INCLUDE FINALS: %S\n", out2.c_str());
                    }
                    reached.push_back(*l);
                  }
                }
              }
              newseq.second = reached;
              new_best_cover = newseq;
              covers[pos] = newseq;
              if(debugMode)
              {
                fwprintf(stderr, L"++ FINALS(%d) covers[%d] [%d, %d] BEST: %.4f > %.4f\n", newseq.second.size(), (pos - path.size()), pos, path.size(), newseq.first, new_best_cover.first);
              }
            }

            last_final = pos;
          }
        }
      }

      alive_states.swap(new_states);
      alive_states.push_back(*initial_state);

      if(debugMode)
      {
        fwprintf(stderr, L"#CURRENT_ALIVE: %d\n", alive_states.size());
      }

      if(alive_states.size() == 1)
      {
        // If we have only a single alive state, it means no rules are
        // active, and we can flush the buffers.
        processFlush(output, sl, tl, blanks, covers, empty_seq, spans, last_final);

        pos = 0;
        last_final = 0;
        tl.clear();
        sl.clear();
        blanks.clear();
        spans.clear();
      }

      pos++;
      if(debugMode)
      {
        fwprintf(stderr, L"==> new pos: %d\n", pos);
      }

      outOfWord = true;
      continue;
    }


    // We're reading a tag
    if(val == L'<' && !isEscaped && !outOfWord)
    {
      wstring tag = L"";
      tag = readFullBlock(input, L'<', L'>');
      sl[pos] = sl[pos] + tag;
      val = static_cast<int>(alphabet(tag));
      if(val == 0)
      {
        val = static_cast<int>(alphabet(L"<ANY_TAG>"));
      }
      if(debugMode)
      {
        fwprintf(stderr, L"tag %S: %d\n", tag.c_str(), val);
      }
    }

    if(!outOfWord)
    {
      if(debugMode)
      {
        fwprintf(stderr, L"outOfWord = false\n");
      }

      new_states.clear();
      wstring res = L"";
      for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
      {
        res = L"";
        State s = *it;
        if(val < 0)
        {
          alphabet.getSymbol(res, val,  false);
          if(debugMode)
          {
            fwprintf(stderr, L"  step: %S\n", res.c_str());
          }
          s.step(val, alphabet(L"<ANY_TAG>"));
        }
        else
        {
          if(debugMode)
          {
            fwprintf(stderr, L"  step: %C\n", val);
          }
          s.step_case(val, alphabet(L"<ANY_CHAR>"), false);
        }
        if(s.size() > 0) // If the current state has outgoing transitions, add it to the new alive states
        {
          new_states.push_back(s);
        }
      }
      if(debugMode)
      {
        fwprintf(stderr, L"new_states: %d\n", new_states.size());
      }
      alive_states.swap(new_states);
      alive_states.push_back(*initial_state);

    }

    // We're still reading a surface form
    if(val > 0 && val != L'$' && !isEscaped && !outOfWord)
    {
      sl[pos] = sl[pos] + static_cast<wchar_t>(val);
    }

    // Reading a superblank
    if(outOfWord)
    {
      if(!feof(input))
      {
        blanks[pos] = blanks[pos] + static_cast<wchar_t>(val);
      }
      if(debugMode)
      {
        //fwprintf(stderr, L"blanks[%d] = %S\n", pos, blanks[pos].c_str());
      }
    }

    // Increment the current line number (for rule tracing)
    if(val == L'\n')
    {
      lineno++;
    }
  }

  processFlush(output, sl, tl, blanks, covers, empty_seq, spans, last_final);

  fwprintf(output, L"%S", blanks[pos].c_str());
}
*/

void
LRXProcessor::process(FILE *input, FILE *output)
{
  bool isEscaped = false;

  map<int, wstring > sl; // map of SL words
  map<int, vector<wstring> > tl; // map of vectors of TL translations
  map<int, wstring > blanks; // map of the superblanks

  map<int, map<wstring, double> > scores; //
  map<int, map<wstring, OpType> > operations;

  vector<State*> alive_states ;
  alive_states.push_back(new State(*initial_state));

  int val = 0;
  while((val = fgetwc_unlocked(input)) != EOF && val != WEOF)
  {

    if(nullFlush && val == L'\0')
    {
      processFlush(output, sl, tl, blanks, scores, operations);
      fwprintf(output, L"%S", blanks[pos].c_str());
      pos = 0;
      tl.clear();
      sl.clear();
      blanks.clear();

      fputwc_unlocked(val, output);
      fflush(output);
      continue;
    }

    // We're starting to read a new lexical form
    if(val == L'^' && !isEscaped && outOfWord)
    {
      outOfWord = false;
      continue;
    }

    // We've seen the surface form
    if(val == L'/' && !isEscaped && !outOfWord)
    {
      // Read in target equivalences
      wstring trad = L"";
      val = fgetwc_unlocked(input);
      while(val != L'$' && val != EOF && val != WEOF)
      {
        if(val != L'$')
        {
          trad += static_cast<wchar_t>(val);
        }
        if(val == L'/')
        {
          tl[pos].push_back(trad.substr(0, trad.length()-1));
          trad = L"";
        }
        val = fgetwc_unlocked(input);
      }
      tl[pos].push_back(trad);

      if(debugMode)
      {
        for(auto& it : tl[pos])
        {
          fwprintf(stderr, L"trad[%d]: %S\n", pos, it.c_str());
        }
      }
    }

    if((feof(input) || val == L'$') && !isEscaped && !outOfWord)
    {
      if(debugMode)
      {
        fwprintf(stderr, L"[POS] %d: [sl %d ; tl %d ; bl %d]: %S\n", pos, sl[pos].size(), tl[pos].size(), blanks[pos].size(), sl[pos].c_str());
      }
      {
        vector<State *> new_states; // TODO: Can we avoid the State-copying here?
        // \forall s \in A
        set<wstring> seen_ids;
        for(auto& it : alive_states)
        {
          State s = *it;
          // \IF \exists c \in Q : \delta(s, sent[i]) = c
          s.step(alphabet(L"<$>"));

          // A \gets A \cup {c}
          if (s.size() > 0) // If the current state has outgoing transitions,
                            // add it to the new alive states
          {
            new_states.push_back(new State(s));
          }
          s.step(alphabet(L"<$>"));

          // \IF c \in F
          if (s.isFinal(anfinals))
          {
            // We've reached a final state, so we need to evaluate the rule we've matched
            if (debugMode)
            {
              wstring out = s.filterFinals(anfinals, alphabet, escaped_chars);
              fwprintf(stderr, L"    filter_finals: %S\n", out.c_str());
            }

            set<pair<wstring, vector<wstring>>> outpaths;
            outpaths = s.filterFinalsLRX(anfinals, alphabet, escaped_chars, false, false, 0);

            for (auto& it : outpaths)
            {
              vector<State> reached;
              vector<wstring> path = it.second;
              wstring id = it.first;

              if (seen_ids.find(id) != seen_ids.end())
              {
                continue;
              }
              seen_ids.insert(id);

              int j = pos - (path.size() - 1);

              if (debugMode)
              {
                fwprintf(stderr, L"id:      %S: (lambda: %.5f)\n", id.c_str(), weights[id.c_str()]);
              }
              for (auto& it2 : path)
              {
                if (debugMode)
                {
                  fwprintf(stderr, L"op:        %S\n", it2.c_str());
                }
                if (it2 != LRX_PROCESSOR_TAG_SKIP)
                {
                  if (scores[j].count(it2) == 0)
                  {
                    scores[j][it2] = 0.0;
                  }
                  scores[j][it2] += weights[id.c_str()];
                  if (debugMode)
                  {
                    fwprintf(stderr, L"#[%d]SCORE %.5f / %S\n", j, scores[j][it2], it2.c_str());
                  }
                  if(it2.at(0) == L'<' && it2.at(1) == L'r') {
                    operations[j][it2] = Remove;
                  }
                  else {
                    operations[j][it2] = Select;
                  }
                }
                j++;
              }
              // fwprintf(stderr, L"#SPAN[%d, %d]\n", (pos-path.size()), pos);
            }
          }
        }
        alive_states.swap(new_states);
        alive_states.push_back(new State(*initial_state));
        for (auto& s : new_states) {
          if (s != initial_state) {
            delete s;
          }
        }

        if (debugMode)
        {
          fwprintf(stderr, L"seen:");
          for (auto& it : seen_ids)
          {
            fwprintf(stderr, L" %S ", it.c_str());
          }
          fwprintf(stderr, L"\n");
          fwprintf(stderr, L"#CURRENT_ALIVE: %d\n", alive_states.size());
        }
      }

      if(alive_states.size() == 1)
      {
        // If we have only a single alive state, it means no rules are
        // active, and we can flush the buffers.

        if(debugMode)
        {
          fwprintf(stderr, L"FLUSH:\n");
        }


        // Here we actually apply the rules that we've matched
        processFlush(output, sl, tl, blanks, scores, operations);

        pos = 0;
        tl.clear();
        sl.clear();
        blanks.clear();
        scores.clear();
        //spans.clear();
      }

      pos++;
      if(debugMode)
      {
        fwprintf(stderr, L"==> new pos: %d\n", pos);
      }

      outOfWord = true;
      continue;
    }

    // We're reading a tag
    if(val == L'<' && !isEscaped && !outOfWord)
    {
      wstring tag = L"";
      tag = readFullBlock(input, L'<', L'>');
      sl[pos] = sl[pos] + tag;
      val = static_cast<int>(alphabet(tag));
      if(val == 0)
      {
        val = static_cast<int>(alphabet(L"<ANY_TAG>"));
      }
      if(debugMode)
      {
        fwprintf(stderr, L"tag %S: %d\n", tag.c_str(), val);
      }
    }

    if(!outOfWord)
    {
      if(debugMode)
      {
        fwprintf(stderr, L"outOfWord = false\n");
      }

      wstring res = L"";
      for(auto& s : alive_states)
      {
        res = L"";
        if(val < 0)
        {
          alphabet.getSymbol(res, val,  false);
          if(debugMode)
          {
            fwprintf(stderr, L"  step: %S\n", res.c_str());
          }
          s->step(val, alphabet(L"<ANY_TAG>"));
        }
        else
        {

          set<int> alts;
          alts.insert(alphabet(L"<ANY_CHAR>"));
          if(iswupper(val))
          {
            alts.insert(towlower(val));
            alts.insert(alphabet(L"<ANY_UPPER>"));
          }
          else
          {
            alts.insert(alphabet(L"<ANY_LOWER>"));
          }
          if(debugMode)
          {
            fwprintf(stderr, L"  step: %C [alts: %d]\n", val, alts.size());
          }
          s->step(val, alts);
        }
      }
      // In the middle of a word, don't push initial state here cf. https://github.com/apertium/apertium-lex-tools/issues/19
    }

    // We're still reading a surface form
    if(val > 0 && val != L'$' && !isEscaped && !outOfWord)
    {
      sl[pos] = sl[pos] + static_cast<wchar_t>(val);
    }

    // Reading a superblank
    if(outOfWord)
    {
      if(!feof(input))
      {
        blanks[pos] = blanks[pos] + static_cast<wchar_t>(val);
      }
      if(debugMode)
      {
        //fwprintf(stderr, L"blanks[%d] = %S\n", pos, blanks[pos].c_str());
      }
    }

    // Increment the current line number (for rule tracing)
    if(val == L'\n')
    {
      lineno++;
    }

  }

  processFlush(output, sl, tl, blanks, scores, operations);
  fwprintf(output, L"%S", blanks[pos].c_str());
}

void
LRXProcessor::processFlush(FILE *output,
                             map<int, wstring > &sl,
                             map<int, vector<wstring> > &tl,
                             map<int, wstring > &blanks,
                             map<int, map<wstring, double> > &scores,
                             map<int, map<wstring, OpType> > &operations) {

  struct ScoredMatch {
      OpType op;
      wstring* ti;              // matched target translation
      double weight;
  };

  unsigned int spos = 0;
  for(spos = 0; spos <= pos; spos++)
  {
    if(sl[spos] == L"")
    {
      continue;
    }

    fwprintf(output, L"%S^%S/", blanks[spos].c_str(), sl[spos].c_str());

    vector<wstring>::iterator ti;
    auto penum = tl[spos].end();
    penum--;

    if(tl[spos].size() > 1)
    {
      //--
      set<wstring*> ti_keep;
      set<wstring*> ti_removed;
      vector<ScoredMatch> spos_matches;
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        ti_keep.insert(&*ti);
        for(const auto& si : scores[spos]) {
          bool matched = recognisePattern(*ti, si.first);
          OpType op = operations[spos][si.first];
          if (debugMode) {
            wstring checks = matched ? L"✔️ " : L"❎";
            fwprintf(stderr, L"%S >>> %d -> %S -> %.5f\n", checks.c_str(), spos,
                     si.first.c_str(), si.second);
          }
          if(matched) {
            spos_matches.push_back({ op, &*ti, si.second });
          }
        }
      }
      if(!spos_matches.empty())  // If we actually got a winner
      {
        sort(spos_matches.begin(),
             spos_matches.end(),
             [](const auto &a, const auto &b) { return a.weight > b.weight; });
        for (const auto &m : spos_matches) {
          if (traceMode || debugMode) {
            wstring op = (m.op == Select ? L"SELECT" : L"REMOVE");
            fwprintf(
                stderr, L"%d:%S:%.5f:%S:%d:%S\n",
                lineno,
                op.c_str(),
                m.weight,
                sl[spos].c_str(),
                ti_keep.size(),
                m.ti->c_str());
          }
          // We have to keep track of translations that have been removed so
          // that we don't end up adding back a translation that was removed.
          if (m.op == Select && ti_removed.find(m.ti) == ti_removed.end()) {
            ti_keep.clear();
            ti_keep.insert(m.ti);
            break;
          } else if(ti_keep.size() > 1) {
            ti_keep.erase(m.ti);
            ti_removed.insert(m.ti);
          }
        }
        bool printed = false;
        for(const auto& ti_max : ti_keep) {
          if(printed) {
            fwprintf(output, L"/");
          }
          fwprintf(output, L"%S", ti_max->c_str());
          printed = true;
        }
      }
      else
      {
        for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
        {
          fwprintf(output, L"%S", ti->c_str());
          if(ti != penum)
          {
            fwprintf(output, L"/");
          }
        }
      }
    }
    else
    {
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        fwprintf(output, L"%S", ti->c_str());
        if(ti != penum)
        {
          fwprintf(output, L"/");
        }
      }
    }

    fwprintf(output, L"$");
    if(debugMode)
    {
      fwprintf(output, L"%d", spos);
    }


  }

}
