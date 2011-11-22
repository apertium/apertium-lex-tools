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
  // escaped_chars chars
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

  pool = new Pool<vector<int> >(4, vector<int>(50));

  initial_state = new State(pool);
  current_state = new State(pool);

  traceMode = false;
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
LRXProcessor::streamError()
{
  throw Exception("Error: Malformed input stream.");
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
    len--;
  }

  fwprintf(stderr, L"Patterns: %d, Alphabet: %d\n", patterns.size(), alphabet.size());

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

wchar_t
LRXProcessor::readEscaped(FILE *input)
{
  if(feof(input))
  {
    streamError();
  }

  wchar_t val = static_cast<wchar_t>(fgetwc_unlocked(input));

  if(feof(input) || escaped_chars.find(val) == escaped_chars.end())
  {
    streamError();
  }

  return val;
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
LRXProcessor::skipUntil(FILE *input, FILE *output, wint_t const character)
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
LRXProcessor::readGeneration(FILE *input, FILE *output)
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
LRXProcessor::applyRules(map<int, SItem> sentence, FILE *output)
{
  int j = 1;
  int k = 1;
  map< pair<int, int>, vector<int> > rule_spans;
  current_state = initial_state;

  for(map<int, SItem>::iterator it = sentence.begin(); it != sentence.end(); it++)
  {
    vector<int> rules;
    fwprintf(stderr, L"%d %S %d\n", it->first, it->second.sl.c_str(), it->second.tl.size());

    while(current_state->size() != 0) 
    {
      if(current_state->isFinal(anfinals))
      {
        wstring out = current_state->filterFinals(anfinals, alphabet, escaped_chars);
 
        fwprintf(stderr, L"%d->%d : %S\n", j, k, out.c_str());
  
        pair<int, int> span = make_pair(j, k);
        rule_spans[span] = rules;
        j = k; 
        rules.clear();
      }

      current_state->step(sentence[k].sl, patterns, alphabet, stderr);
      k++;
    }
    j++;
  }

  
  for(map<int, SItem>::iterator it = sentence.begin(); it != sentence.end(); it++)
  {
    fwprintf(output, L"%S^%S", it->second.blank.c_str(), it->second.sl.c_str());
    set<wstring> tl = it->second.tl;
    for(set<wstring>::iterator it2 = tl.begin(); it2 != tl.end(); it2++) 
    { 
      if(it2 != tl.end()) 
      {
        fwprintf(output, L"/");
      }
      fwprintf(output, L"%S", it2->c_str());
    }
    fwprintf(output, L"$ ");
  }

  return;
}

void
LRXProcessor::process(FILE *input, FILE *output)
{
  map<int, SItem> sentence;   // pos, item
  bool seenFirst = false;
  int pos = 0;
  int val = 0;
 
  wstring sl = L""; // the current SL side
  wstring tl = L""; // the current SL side
  set<wstring> tlv;  // the current TL side

  skipUntil(input, output, L'^');
  outOfWord = false;
  pos++;
 
  while((val = readGeneration(input, output)) != 0x7fffffff) 
  {
    //fwprintf(stderr, L"%C\n", val);
    switch(val)
    {
      case L'/':
        if(!seenFirst)
        {
          seenFirst = true;
        }
        else
        {
          tlv.insert(tl);
        }
        tl = L"";
        val = readGeneration(input, output);
        if(val != L'$')
        {
          break;
        }
      case L'$':
        tlv.insert(tl);
        sentence[pos].sl = sl;
        sentence[pos].tl = tlv;
        if(sl.compare(LRX_PROCESSOR_S_BOUNDARY) == 0)
        {
          applyRules(sentence, output);
          sentence.clear();
          pos = 0;
        }
        seenFirst = false;
        sl = L"";
        tl = L"";
        tlv.clear();
        pos++;
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
    else if(outOfWord && val != L'$')
    {
      sentence[pos].blank.append(1, static_cast<wchar_t>(val));
    }
  }
  
  return;
}


