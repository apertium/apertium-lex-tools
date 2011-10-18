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
#include <lttoolbox/trans_exe.h>

using namespace std;

#define PACKAGE_VERSION "0.1.0"

map<wstring, TransExe> transducers; 

int main (int argc, char** argv)
{
  Alphabet alphabet;
  Transducer t;
  TransExe te;
  map<int, Transducer> patterns;
  map<wstring, Transducer> transducers;

  LtLocale::tryToSetLocale();

  set<wchar_t> escaped;
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
    transducers[name].read(fst);
    len--;
  }
  fwprintf(ous, L"Patterns: %d, Alphabet: %d\n", transducers.size(), alphabet.size());

  for(map<wstring, Transducer>::iterator it = transducers.begin(); it != transducers.end(); it++)
  {
    fwprintf(ous, L"= %S =============================\n", it->first.c_str());
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
  State *initial_state = new State(pool);
  initial_state->init(te.getInitial());
  State current_state = *initial_state;

  wstring input, output=L"";

  set<Node *> anfinals;
  anfinals.insert(te.getFinals().begin(), te.getFinals().end());

  wstring v= L"";
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
      current_state.step(alphabet(v));
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

  return 0;
}
