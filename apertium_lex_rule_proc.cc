#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <list>
#include <set>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/alphabet.h>
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



//map<wstring, TransExe> transducers; 
Alphabet alphabet;
State *initial_state;
set<Node *> anfinals;
set<wchar_t> escaped;
bool outOfWord = true;
map<int, Transducer> transducers;


typedef struct SL 
{
  int pos;
  wstring lu;
} SL;

// {0, prova<n><f><sg>: [proof<n><sg>, event<n><sg>, exam<n><sg>, trial<n><sg>, test<n><sg>]}
map< pair<int, wstring>, vector<wstring> > sentence;





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
        fputws_unlocked(L"^", ous);
        fputws_unlocked(sl.c_str(), ous);
        int j = 0;
        for(set<wstring>::const_iterator it = tllu.begin(), j = tllu.end(); it != j; it++)
        {
          if(it != tllu.end())
          {
            fputws_unlocked(L"/", ous);
          }
          wstring t = *it;
          fputws_unlocked(t.c_str(), ous);
          sentence[cur_sl].push_back(t);
        }
        fputws_unlocked(L"$", ous);

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

    fwprintf(ous, L"%d %S: %d\n", sl_pair.first, sl_pair.second.c_str(), tl_lloc.size());
    if(current_state.size() == 0) 
    {
      cur_pos = sl_pair.first;
      current_state = *initial_state;
    }
    current_state.step(sl_pair.second, transducers, alphabet, ous);
    if(current_state.isFinal(anfinals))
    {
      wstring out = current_state.filterFinals(anfinals, alphabet, escaped);
      fwprintf(ous, L"FINAL: %d %S: %d\n", sl_pair.first, sl_pair.second.c_str(), tl_lloc.size());
      fwprintf(ous, L"Path: %S\n", out.c_str());
      operations[cur_pos] = out;
    }
  }

  fwprintf(ous, L"\n");
  for(map< int, wstring>::iterator it2 = operations.begin(); it2 != operations.end(); it2++)
  {
    fwprintf(ous, L"%d -> %S\n", it2->first, it2->second.c_str());
  }

/*
  wchar_t c = (wchar_t)fgetwc(in);
  while (c != WEOF)
  {
    if(iswspace(c))
    {
      v = L"<" + v + L">";
      if(!alphabet.isSymbolDefined(v))
      {
        fwprintf(ous, L"pattern: %S not defined in alphabet\n", v.c_str());
      }
      //current_state.step(v, alphabet(v));
      input.append(v);
      v = L"";
      input = input + c;
      //wstring x = current_state.getReadableString(alphabet);
      //fwprintf(ous, L"grs: %S\n", x.c_str());
    }
    else
    {
        v = v + c; 
    }
    c = (wchar_t)fgetwc(in);
  }
  if (current_state.isFinal(anfinals))
  {
    output = current_state.filterFinals(anfinals, alphabet, escaped);
    wcout << endl << input << endl;
    wcout << output.substr(1, -1) << endl;
  }
  else
  {
    wcout << endl << L"\nUnrecognised: " << input << endl;
  }
 

  for(map< pair<int, wstring>, vector<wstring> >::iterator it = sentence.begin(); it != sentence.end(); it++)
  {
    pair<int, wstring> sl = it->first;
    vector<wstring> tl = it->second;

    //step;
  } 
*/

}


int main (int argc, char** argv)
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

  fwprintf(ous, L"%d\n", len);

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
  fwprintf(ous, L"Patterns: %d, Alphabet: %d\n", transducers.size(), alphabet.size());

  for(map<int, Transducer>::iterator it = transducers.begin(); it != transducers.end(); it++)
  {
    wstring sym;
    alphabet.getSymbol(sym, it->first, false);
    fwprintf(ous, L"= %d (%d) =============================\n", it->first, it->second.size());
    fwprintf(ous, L"  %S\n", sym.c_str());
    it->second.show(alphabet, ous);
  }

  int len3 = Compression::multibyte_read(fst);
  wstring name = L"";
  while(len3 > 0)
  {
      name += static_cast<wchar_t>(Compression::multibyte_read(fst));
      len3--;
  }
  wcout << name << endl;
  te.read(fst, alphabet);
  //t.show(alphabet, ous); 

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
