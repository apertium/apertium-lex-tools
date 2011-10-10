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

  FILE *ins = fopen(argv[1], "r");
  FILE *ous = stdout;

  wstring rule = L"";
  int val = 0;
  val = fgetwc_unlocked(ins);
  while(val != EOF) {
     if(val == L'\n') 
     {
       wchar_t c;
       wstring tipus = L"";
       wstring sl = L"";
       wstring tl = L"";

       int i = 0; 
       while(rule.at(i) != '\t')                                     // First read the rule type
       {
         tipus = tipus + rule.at(i);
         i++;
       }
       i++;
       c = rule.at(i);
       bool inq = false;
       bool inp = false;
       int seen = 0;

       while(c != L'\t')                                             // Then the source word
       {
         if(c == L'(' || c == L')')  
         {
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 0 && c == L'"' && inq == false) 
         {
           seen = 1; 
           inq = true;
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 1 && c == L'"' && inq == true) 
         {
           seen = 2; 
           inq = false;
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 2 && c == L' ') 
         { 
           sl = sl + L'<'; 
           seen = 3;
           i++;
           c = rule.at(i); 
           continue;
         }
         if(seen == 3 && c == L' ') 
         {
           sl = sl + L"><";
           i++;
           c = rule.at(i);
           continue; 
         } 
         sl = sl + c;
         i++;
         c = rule.at(i);
       }
       sl = sl + L">.*";
       i++;
       inq = false;
       inp = false;
       seen = 0;
       c = rule.at(i);
       while(c != L'\t')                                             // Then the target word
       {
         if(c == L'(' || c == L')')  
         {
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 0 && c == L'"' && inq == false) 
         {
           seen = 1; 
           inq = true;
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 1 && c == L'"' && inq == true) 
         {
           seen = 2; 
           inq = false;
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 2 && c == L' ') 
         { 
           tl = tl + L'<'; 
           seen = 3;
           i++;
           c = rule.at(i); 
           continue;
         }
         if(seen == 3 && c == L' ') 
         {
           tl = tl + L"><";
           i++;
           c = rule.at(i);
           continue; 
         } 
         tl = tl + c;
         i++;
         c = rule.at(i);
       }
       tl = tl + L">.*";
       i++;

       map<int, pair<wstring, wstring> > context;
       pair<wstring, wstring> td = pair<wstring, wstring>(sl, tl);
       context[0] = td;
       wstring pos = L"", lem = L"", tag = L"";

       inq = false;
       inp = false;
       seen = 0;
       c = rule.at(i);
       while(i < (rule.size() - 1))                                             // Then the context 
       {
         //fwprintf(ous, L"%d %d %c\n ", i, rule.size(), c);
         if(c == L'(') 
         { 
           seen = 0;
           pos = L"", lem = L"", tag = L"";
           inp = true;
           i++; 
           c = rule.at(i);
           continue;
         }
         else if(c == L')') 
         { // read one
           wistringstream wstrm(pos);
           int p = -1;
           wstrm >> p;
           wstring csl;
           if(tag != L"") 
           {
             csl = lem + L"<" + tag + L">.*";
           } 
           else
           {
             csl = lem;
           }
           pair<wstring, wstring> td = pair<wstring, wstring>(csl, L"*");
           context[p] = td;
           inp = false;
           inq = false;
           i++; 
           c = rule.at(i);
           continue;
         }
         else if(c == L'"' and inq == false) 
         { 
           seen = 1;
           inq = true;
           i++; 
           c = rule.at(i);
           continue;
         }
         else if(c == L'"' and inq == true) 
         { 
           seen = 2;
           inq = false;
           i++; 
           c = rule.at(i);
           continue;
         }

         switch(seen) 
         {
           case 0:
              pos = pos + c;
              i++;
              c = rule.at(i);
              continue;
           case 1: 
              if(c == L'*') 
              {
                lem = lem + L"[\\w ]+";
              } 
              else 
              {
                lem = lem + c;
              }
              i++;
              c = rule.at(i);
              continue;
           case 2:
              if(c == L' ')
              { 
                seen = 3;
              } 
              else
              {
                tag = tag + c;
              }
              i++;
              c = rule.at(i);
              continue;
           case 3:
              if(c == L' ')
              { 
                tag = tag + L"><";
              }
              else
              { 
                tag = tag + c;
              }
              i++;
              c = rule.at(i);
              continue;
  
         }
         i++;
         c = rule.at(i);
       }
       
       wistringstream wstrm(pos);
       int p = 1001;
       wstrm >> p;
       wstring csl;
       if(tag != L"") 
       {
         csl = lem + L"<" + tag + L">.*";
       } 
       else
       {
         csl = lem;
       }
       td = pair<wstring, wstring>(csl, L"*");
       context[p] = td;
       inp = false;
       pos = L"", lem = L"", tag = L"";

       fputws_unlocked(rule.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" tipus: ", ous);
       fputws_unlocked(tipus.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" sl: ", ous);
       fputws_unlocked(sl.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" tl: ", ous);
       fputws_unlocked(tl.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" context: ", ous);
       fputws_unlocked(L"\n", ous);
       int s = t.getInitial();
       for(map<int, pair<wstring, wstring> >::iterator it = context.begin();  
                 it != context.end(); it++)
       {
         fputws_unlocked(L"  ", ous);
         int pos = it->first;
         pair<wstring, wstring> pat = it->second;
         fwprintf(ous, L"%d ", pos);
         fputws_unlocked(pat.first.c_str(), ous);
         fputws_unlocked(L":", ous);
         fputws_unlocked(pat.second.c_str(), ous);
         fputws_unlocked(L"\n", ous);

         if(!alphabet.isSymbolDefined(pat.first.c_str()))
         {
           alphabet.includeSymbol(pat.first.c_str());  
         }
         if(!alphabet.isSymbolDefined(pat.second.c_str()))
         {
           alphabet.includeSymbol(pat.second.c_str());  
         }

         s = t.insertSingleTransduction(alphabet(alphabet(pat.first.c_str()), alphabet(pat.second.c_str())), s);
       }
       t.setFinal(s);
       fputws_unlocked(L"\n", ous);
       fwprintf(ous, L"%d@%d %d\n", t.size(), t.numberOfTransitions(), alphabet.size());

       rule = L"";
     }
     else
     {
       rule.append(1, static_cast<wchar_t>(val));
     }
     val = fgetwc_unlocked(ins);
  } 

  t.minimize();
  fwprintf(ous, L"%d@%d %d\n", t.size(), t.numberOfTransitions(), alphabet.size());

  FILE* fst=fopen(argv[2], "w");
  alphabet.write(fst);
  t.write(fst);
  fclose(fst);

  return 0;
}
