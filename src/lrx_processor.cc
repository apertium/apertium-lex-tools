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

#include <weight.h>
#include <lrx_processor.h>
#include <iostream>
#include <algorithm>
#include <lttoolbox/compression.h>

using namespace std;

UString const LRXProcessor::LRX_PROCESSOR_TAG_SELECT     = "<select>"_u;
UString const LRXProcessor::LRX_PROCESSOR_TAG_REMOVE     = "<remove>"_u;
UString const LRXProcessor::LRX_PROCESSOR_TAG_SKIP       = "<skip>"_u;

UString const LRXProcessor::LRX_PROCESSOR_TAG_ANY_CHAR       = "<ANY_CHAR>"_u;
UString const LRXProcessor::LRX_PROCESSOR_TAG_ANY_TAG        = "<ANY_TAG>"_u;
UString const LRXProcessor::LRX_PROCESSOR_TAG_ANY_UPPER      = "<ANY_UPPER>"_u;
UString const LRXProcessor::LRX_PROCESSOR_TAG_ANY_LOWER      = "<ANY_LOWER>"_u;
UString const LRXProcessor::LRX_PROCESSOR_TAG_WORD_BOUNDARY  = "<$>"_u;

UString
LRXProcessor::itow(int i)
{
  // Convert an int to a UString
  UChar buf[50];
  u_snprintf(buf, 50, "%d", i);
  UString id(buf);
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
  any_char      = alphabet(LRX_PROCESSOR_TAG_ANY_CHAR);
  any_tag       = alphabet(LRX_PROCESSOR_TAG_ANY_TAG);
  any_upper     = alphabet(LRX_PROCESSOR_TAG_ANY_UPPER);
  any_lower     = alphabet(LRX_PROCESSOR_TAG_ANY_LOWER);
  word_boundary = alphabet(LRX_PROCESSOR_TAG_WORD_BOUNDARY);

  int len = Compression::multibyte_read(in);

  while(len > 0)
  {
    UString name = Compression::string_read(in);
    recognisers[name].read(in, alphabet);
    if(debugMode)
    {
      cerr << "Recogniser: " << name << ", [finals: " << recognisers[name].getFinals().size() << "]\n";
    }
    len--;
  }

  if(debugMode)
  {
    cerr << "recognisers: " << recognisers.size() << endl;
  }

  UString name = Compression::string_read(in);

  transducer.read(in, alphabet);

  // Now read in weights
  weight record;
  while(fread(&record, sizeof(weight), 1, in))
  {
    weight_from_le(record);
    UString sid = "<"_u + itow(record.id) + ">"_u;
    weights[sid] = record.pisu;

    /*
    if(debugMode)
    {
      cerr << sid << " " << record.id << " weight(" << record.pisu << ")\n";
    }
    */
  }

  return;
}

void
LRXProcessor::init()
{
  initial_state->init(transducer.getInitial());

  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());

  escaped_chars.insert('[');
  escaped_chars.insert(']');
  escaped_chars.insert('{');
  escaped_chars.insert('}');
  escaped_chars.insert('^');
  escaped_chars.insert('$');
  escaped_chars.insert('/');
  escaped_chars.insert('\\');
  escaped_chars.insert('@');
  escaped_chars.insert('<');
  escaped_chars.insert('>');

}

bool
LRXProcessor::recognisePattern(const UString lu, const UString op)
{
  if(recognisers.count(op) < 1)
  {
    cerr << "WARNING: Recogniser not found for key " << op << ", skipping... [LU: " << lu << "]" << endl;
    return false;
  }

  State *first_state = new State();
  first_state->init(recognisers[op].getInitial());
  State cur = *first_state;

  map<Node *, double> end_states;
  end_states.insert(recognisers[op].getFinals().begin(), recognisers[op].getFinals().end());

  bool readingTag = false;
  UString tag;
  int val = 0;
  for(auto& it : lu)
  {
/*
    if(debugMode)
    {
      cerr << "alive: " << cur.size() << endl;
    }
*/
    if(cur.size() < 1)  // I think that any time we have 0 alive states,
                        // we can say that the string is unrecognised
    {
      return false;
    }
    if(it == '<')
    {
      tag.clear();
      readingTag = true;
      tag += it;
      continue;
    }
    if(it == '>')
    {
      tag = tag + it;
      val = alphabet(tag);
      if(val == 0)
      {
        val = any_tag;
      }
/*
      if(debugMode)
      {
        cerr << ":: tag " << tag << ": " << val << endl;
        cerr << "  step: " << tag << endl;
      }
*/
      cur.step(val, any_tag);
      readingTag = false;
      continue;
    }
    if(readingTag)
    {
      tag += it;
    }
    else
    {
      // We're not in a tag, so were just reading characters
      int val = static_cast<int>(it);
/*
      if(debugMode)
      {
        cerr << "  step: " << val << endl;
      }
*/
      //cur.step(val, a("<ANY_CHAR>"));
      //cur.step(val);
      set<int> alts;
      alts.insert(any_char);
      if(!u_isupper(val))
      {
        alts.insert(any_lower);
      }
      else
      {
        alts.insert(any_upper);
        alts.insert(u_tolower(val));
      }
      cur.step(val, alts);

    }
  }

/*
  if(debugMode)
  {
    cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
  }
*/
  if(cur.isFinal(end_states))
  {
    return true;
  }

  return false;
}

void
LRXProcessor::process(InputFile& input, UFILE *output)
{
  bool isEscaped = false;

  map<int, UString > sl; // map of SL words
  map<int, vector<UString> > tl; // map of vectors of TL translations
  map<int, UString > blanks; // map of the superblanks

  map<int, map<UString, double> > scores; //
  map<int, map<UString, OpType> > operations;

  vector<State*> alive_states ;
  alive_states.push_back(new State(*initial_state));

  int32_t val = 0;
  while((val = input.get()) != U_EOF)
  {

    if(nullFlush && val == '\0')
    {
      processFlush(output, sl, tl, blanks, scores, operations);
      u_fprintf(output, "%S", blanks[pos].c_str());
      pos = 0;
      tl.clear();
      sl.clear();
      blanks.clear();
      scores.clear();
      operations.clear();
      alive_states.clear();
      alive_states.push_back(new State(*initial_state));

      u_fputc(val, output);
      u_fflush(output);
      continue;
    }

    // We're starting to read a new lexical form
    if(val == '^' && !isEscaped && outOfWord)
    {
      outOfWord = false;
      continue;
    }

    // We've seen the surface form
    if(val == '/' && !isEscaped && !outOfWord)
    {
      // Read in target equivalences
      UString trad;
      val = input.get();
      while(val != '$' && val != U_EOF)
      {
        if(val != '$')
        {
          trad += val;
        }
        if(val == '/')
        {
          tl[pos].push_back(trad.substr(0, trad.length()-1));
          trad.clear();
        }
        val = input.get();
      }
      tl[pos].push_back(trad);

      if(debugMode)
      {
        for(auto& it : tl[pos]) {
          cerr << "trad[" << pos << "]: " << it << endl;
        }
      }
    }

    if((input.eof() || val == '$') && !isEscaped && !outOfWord)
    {
      if(debugMode)
      {
        cerr << "[POS] " << pos << ": [sl " << sl[pos].size() << " ; tl " << tl[pos].size() << " ; bl " << blanks[pos].size() << "]: " << sl[pos] << endl;
      }
      {
        vector<State *> new_states; // TODO: Can we avoid the State-copying here?
        // \forall s \in A
        set<UString> seen_ids;
        for(auto& it : alive_states)
        {
          State s = *it;
          // \IF \exists c \in Q : \delta(s, sent[i]) = c
          s.step(word_boundary);

          // A \gets A \cup {c}
          if (s.size() > 0) // If the current state has outgoing transitions,
                            // add it to the new alive states
          {
            new_states.push_back(new State(s));
          }
          s.step(word_boundary);

          // \IF c \in F
          if (s.isFinal(anfinals))
          {
            // We've reached a final state, so we need to evaluate the rule we've matched
            if (debugMode)
            {
              UString out = s.filterFinals(anfinals, alphabet, escaped_chars);
              cerr << "    filter_finals: " << out << endl;
            }

            set<pair<UString, vector<UString>>> outpaths;
            outpaths = s.filterFinalsLRX(anfinals, alphabet, escaped_chars, false, false, 0);

            for (auto& it : outpaths)
            {
              vector<State> reached;
              vector<UString> path = it.second;
              UString id = it.first;

              if (seen_ids.find(id) != seen_ids.end())
              {
                continue;
              }
              seen_ids.insert(id);

              int j = pos - (path.size() - 1);

              if (debugMode)
              {
                cerr << "id:      " << id << ": (lambda: ";
                cerr << weights[id] << ")\n";
              }
              for (auto& it2 : path)
              {
                if (debugMode)
                {
                  cerr << "op:        " << it2 << endl;
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
                    cerr << "#[" << j << "]SCORE " << scores[j][it2] << " / ";
                    cerr << it2 << endl;
                  }
                  if(it2.at(0) == '<' && it2.at(1) == 'r') {
                    operations[j][it2] = Remove;
                  }
                  else {
                    operations[j][it2] = Select;
                  }
                }
                j++;
              }
              // cerr << "#SPAN[" << (pos-path.size()) << ", " << pos << "]\n";
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
          cerr << "seen:";
          for (auto& it : seen_ids) {
            cerr << " " << it << " ";
          }
          cerr << endl;
          cerr << "#CURRENT_ALIVE: " << alive_states.size() << endl;
        }
      }

      if(alive_states.size() == 1)
      {
        // If we have only a single alive state, it means no rules are
        // active, and we can flush the buffers.

        if(debugMode)
        {
          cerr << "FLUSH:" << endl;
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
        cerr << "==> new pos: " << pos << endl;
      }

      outOfWord = true;
      continue;
    }

    // We're reading a tag
    if(val == '<' && !isEscaped && !outOfWord)
    {
      UString tag = input.readBlock('<', '>');
      sl[pos] = sl[pos] + tag;
      val = alphabet(tag);
      if (val == 0) {
        val = any_tag;
      }
      if(debugMode)
      {
        cerr << "tag " << tag << ": " << val << "\n";
      }
    }

    if(!outOfWord)
    {
      if(debugMode)
      {
        cerr << "outOfWord = false\n";
      }

      UString res;
      for(auto& s : alive_states)
      {
        res.clear();
        if(val == '*' && sl[pos].empty()) {
          if(debugMode) {
            cerr << "  skipping unknown marker" << endl;
          }
        }
        else if(val < 0)
        {
          alphabet.getSymbol(res, val,  false);
          if(debugMode)
          {
            cerr << "  step: " << res << endl;
          }
          s->step(val, any_tag);
        }
        else
        {

          set<int> alts;
          alts.insert(any_char);
          if(u_isupper(val))
          {
            alts.insert(u_tolower(val));
            alts.insert(any_upper);
          }
          else
          {
            alts.insert(any_lower);
          }
          if(debugMode)
          {
            cerr << "  step: " << val << " [alts: " << alts.size() << "]\n";
          }
          s->step(val, alts);
        }
      }
      // In the middle of a word, don't push initial state here cf. https://github.com/apertium/apertium-lex-tools/issues/19
    }

    // We're still reading a surface form
    if(val > 0 && val != '$' && !isEscaped && !outOfWord)
    {
      sl[pos] += val;
    }

    // Reading a superblank
    if(outOfWord)
    {
      if(!input.eof())
      {
        blanks[pos] += val;
      }
      /*
      if(debugMode)
      {
        cerr << "blanks[" << pos << "] = " << blanks[pos] << endl;
      }
      */
    }

    // Increment the current line number (for rule tracing)
    if(val == '\n')
    {
      lineno++;
    }

  }

  processFlush(output, sl, tl, blanks, scores, operations);
  write(blanks[pos], output);
}

void
LRXProcessor::processFlush(UFILE *output,
                           map<int, UString > &sl,
                           map<int, vector<UString> > &tl,
                           map<int, UString > &blanks,
                           map<int, map<UString, double> > &scores,
                           map<int, map<UString, OpType> > &operations) {

  struct ScoredMatch {
      OpType op;
      UString* ti;              // matched target translation
      double weight;
  };

  unsigned int spos = 0;
  for(spos = 0; spos <= pos; spos++)
  {
    if(sl[spos].empty())
    {
      continue;
    }

    u_fprintf(output, "%S^%S/", blanks[spos].c_str(), sl[spos].c_str());

    vector<UString>::iterator ti;
    auto penum = tl[spos].end();
    penum--;

    if(tl[spos].size() > 1)
    {
      //--
      set<UString*> ti_keep;
      set<UString*> ti_removed;
      vector<ScoredMatch> spos_matches;
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        ti_keep.insert(&*ti);
        for(const auto& si : scores[spos]) {
          bool matched = recognisePattern(*ti, si.first);
          OpType op = operations[spos][si.first];
          if (debugMode) {
            if (matched) {
              cerr << "✔️ ";
            } else {
              cerr << "❎";
            }
            cerr << " >>> " << spos << " -> ";
            cerr << si.first << " -> " << si.second << endl;
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
            std::string op = (m.op == Select ? "SELECT" : "REMOVE");
            cerr << lineno << ":" << op << ":" << m.weight;
            cerr << ":" << sl[spos] << ":" << ti_keep.size();
            cerr << ":" << *m.ti << endl;
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
            u_fprintf(output, "/");
          }
          u_fprintf(output, "%S", ti_max->c_str());
          printed = true;
        }
      }
      else
      {
        for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
        {
          u_fprintf(output, "%S", ti->c_str());
          if(ti != penum)
          {
            u_fprintf(output, "/");
          }
        }
      }
    }
    else
    {
      for(ti = tl[spos].begin(); ti != tl[spos].end(); ti++)
      {
        u_fprintf(output, "%S", ti->c_str());
        if(ti != penum)
        {
          u_fputc('/', output);
        }
      }
    }

    u_fputc('$', output);
    if(debugMode)
    {
      u_fprintf(output, "%d", spos);
    }


  }

}
