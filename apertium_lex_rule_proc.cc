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
#include <lttoolbox/pool.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

using namespace std;


int main (int argc, char** argv)
{
  Alphabet alphabet;
  Transducer t;

  LtLocale::tryToSetLocale();

  FILE* in=stdin;
  FILE* ous=stdout;

  FILE* fst;
  fst=fopen(argv[1], "r");

  TransExe te;
  alphabet.read(fst);
  fwprintf(ous, L"%d\n", alphabet.size());
  te.read(fst, alphabet);
  fclose(fst);

//  exit(-1);

  Pool<vector<int> > *pool = new Pool<vector<int> >(1, vector<int>(50));
  State *initial_state = new State(pool);
  initial_state->init(te.getInitial());
  State current_state = *initial_state;

  wstring input, output=L"";

  set<Node *> anfinals;
  anfinals.insert(te.getFinals().begin(), te.getFinals().end());


  bool reading=true;
  // This is our runtime: see if the input matches
  wstring v= L"";
  wchar_t c = (wchar_t)fgetwc(in);
  while (c != WEOF)
  {
    if(iswspace(c))
    {
      input = input + c;
      fwprintf(ous, L"%S/" , v.c_str());
      current_state.step(alphabet(v));
      input.append(v);
      v = L"";
    }
    else
    {
        v = v + c; 
    }
    c = (wchar_t)fgetwc(in);
  }

  if (current_state.isFinal(anfinals))
  {
    // Not used, just don't want it to be empty...
    set<wchar_t> escaped;
    escaped.insert(L'$');
    output = current_state.filterFinals(anfinals, alphabet, escaped);

    wcout << input << output << endl;
  }
  else
  {
    wcout << L"Unrecognised: " << input << endl;
  }

  return 0;
}
