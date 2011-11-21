#include <cwchar>
#include <cstdio>
#include <libgen.h>
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
#include <lttoolbox/compression.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

#define PACKAGE_VERSION "0.1.0"


using namespace std;

void endProgram(char *name)
{
  if(name != NULL)
  {
    cout << basename(name) << " v" << PACKAGE_VERSION <<": build a selection transducer from a ruleset" << endl;
    cout << "USAGE: " << basename(name) << " rule_file output_file" << endl;
  }
  exit(EXIT_FAILURE);
}

void read_rules(FILE *in)
{

}

int main (int argc, char** argv)
{
  Alphabet alphabet;
  Transducer t;
  map<int, Transducer> patterns;

  LtLocale::tryToSetLocale();

  if(argc < 3) 
  {
    endProgram(argv[0]);
  }

  FILE *ins = fopen(argv[1], "r");
  FILE *fst = fopen(argv[2], "w");
  FILE *ous = stdout;


  wstring rule = L"";
  int val = 0;
  int rule_count = 1;
  val = fgetwc_unlocked(ins);
  // We read the rule file character by character until the end
  while(val != EOF) {
     if(val == L'\n') // One rule per line
     {
       wchar_t c;
       wstring tipus = L""; // The type of rule (s 'select', r 'remove')
       wstring sl = L""; // The source language pattern of the rule 
       wstring tl = L""; // The target language pattern of the rule (that the operation works on)

       unsigned int i = 0; 
       while(rule.at(i) != '\t')                 // First read the rule type
       {
         tipus = tipus + rule.at(i);
         i++;
       }
       i++;
       c = rule.at(i);
       bool inq = false; // In quote marks "
       bool inp = false; // In parentheses (
       int seen = 0; // What have we seen ?

       while(c != L'\t')                         // Then the source word
       {
         if(c == L'(' || c == L')') // Skip parentheses 
         {
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 0 && c == L'"' && inq == false)  
         {
           seen = 1; // We're seeing the lemma
           inq = true;
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 1 && c == L'"' && inq == true) 
         {
           seen = 2; // We've seen the lemma
           inq = false;
           i++;
           c = rule.at(i);
           continue; 
         }
         if(seen == 2 && c == L' ') 
         { 
           sl = sl + L'<'; 
           seen = 3; // We're in the first tag 
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
       sl = sl + L">[0-9A-Za-z <>]*";
       i++;
       inq = false;
       inp = false;
       seen = 0;
       c = rule.at(i);
       while(c != L'\t')                         // Then the target word (this pattern is as above)
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
       tl = tl + L">[0-9A-Za-z <>]*";
       i++;

       map<int, pair<wstring, wstring> > context;
       pair<wstring, wstring> td = pair<wstring, wstring>(sl, tl);
       context[0] = td;
       wstring pos = L"", lem = L"", tag = L"";

       inq = false;
       inp = false;
       seen = 0;
       c = rule.at(i);
       int context_count = 0;
       while(i < (rule.size() - 1))              // Then the context 
       {
         //fwprintf(ous, L"%d %d %c\n ", i, rule.size(), c);
         if(c == L'(') 
         { 
           seen = 0;
           pos = L"", lem = L"", tag = L"";
           inp = true;
           i++; 
           c = rule.at(i);
           context_count++;
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
             csl = lem + L"<" + tag + L">[0-9A-Za-z <>]*";
           } 
           else
           {
             csl = lem + L"<[0-9A-Za-z <>]*";
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
                lem = lem + L"[0-9A-Za-z #]+"; // Replace Kleene star with equivalent in regex
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
       int p = 1001; // The position
       wstrm >> p;
       wstring csl;
       if(tag != L"") 
       {
         csl = lem + L"<" + tag + L">[0-9A-Za-z <>]*";
       } 
       else
       {
         csl = lem;
       }
       td = pair<wstring, wstring>(csl, L"*");
       context[p] = td;
       inp = false;
       pos = L"", lem = L"", tag = L"";

       if(tipus == L"s")
       { 
         tipus = L"select"; 
       }
       else if(tipus == L"r")
       {
         tipus = L"remove"; 
       }
       fputws_unlocked(rule.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" tipus: ", ous);
       fputws_unlocked(tipus.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" sl: ", ous);
       fputws_unlocked(sl.c_str(), ous);
       fputws_unlocked(L"\n", ous);
       fputws_unlocked(L" tl: ", ous);
       fputws_unlocked(tipus.c_str(), ous);
       fputws_unlocked(L"(", ous);
       fputws_unlocked(tl.c_str(), ous);
       fputws_unlocked(L")", ous);
       fputws_unlocked(L"\n", ous);
       fwprintf(ous, L" context (%d): ", context_count);
       fputws_unlocked(L"\n", ous);

       if(!alphabet.isSymbolDefined(L"<*>"))
       {
         alphabet.includeSymbol(L"<*>");  
       }
       if(!alphabet.isSymbolDefined(L"<skip(*)>"))
       {
         alphabet.includeSymbol(L"<skip(*)>");
       }


       int s = t.getInitial();
       map<int, pair<wstring, wstring> >::iterator fpos = context.begin();
       int first_pos = (fpos->first - 1);  
       fwprintf(ous, L" first: %d\n", first_pos);
       for(map<int, pair<wstring, wstring> >::iterator it = context.begin();  
                 it != context.end(); it++)
       {
         int pos = it->first;
 
         pair<wstring, wstring> pat = it->second;
         fwprintf(ous, L"  %d ", pos);
         fputws_unlocked(pat.first.c_str(), ous);
         fputws_unlocked(L":", ous);
         fputws_unlocked(pat.second.c_str(), ous);
         fputws_unlocked(L"\n", ous);

         RegexpCompiler re;
         re.initialize(&alphabet);
         re.compile(pat.first);
         wstring left = L"";
         if(pat.first.find(L"<") == wstring::npos)
         {
           left = L"<" + pat.first + L"<[0-9A-Za-z <>]*>";
         }
         else
         {
           left = L"<" + pat.first + L">";
         }
         wstring right = L"";
         if(pos == 0) 
         {
           right = L"<" + tipus + L"(" + pat.second + L")>";
         } 
         else
         {
           right = L"<skip(" + pat.second + L")>";
         }

         if(!alphabet.isSymbolDefined(left.c_str()))
         {
           alphabet.includeSymbol(left.c_str());  
         }
         if(!alphabet.isSymbolDefined(right.c_str()))
         {
           alphabet.includeSymbol(right.c_str());  
         }
         s = t.insertSingleTransduction(alphabet(alphabet(left.c_str()), alphabet(right.c_str())), s);
         if(patterns.count(alphabet(left.c_str())) < 1) 
         {
           Transducer t = re.getTransducer();
           t.minimize();
           patterns[alphabet(left.c_str())] = t;
         }
       }
       wchar_t rule_count_str[50];
       wchar_t context_count_str[50];
       memset(rule_count_str, '\0', sizeof(rule_count_str));
       memset(context_count_str, '\0', sizeof(rule_count_str));
       swprintf(rule_count_str, 50, L"%d", rule_count);
       swprintf(context_count_str, 50, L"%d", context_count);
       fwprintf(ous, L"rule/context_number: %S %S\n", rule_count_str, context_count_str);
       wstring id(rule_count_str);
       wstring cc(context_count_str);
       id = L"<" + id + L"," + cc + L">";
       if(!alphabet.isSymbolDefined(id.c_str()))
       {
         alphabet.includeSymbol(id.c_str());  
       }

       s = t.insertSingleTransduction(alphabet(0, alphabet(id)), s);

       t.setFinal(s);
       fputws_unlocked(L"\n", ous);
       fwprintf(ous, L"%d@%d %d\n", t.size(), t.numberOfTransitions(), alphabet.size());

       rule = L"";
 
       rule_count++;
     }
     else
     {
       rule.append(1, static_cast<wchar_t>(val));
     }
     val = fgetwc_unlocked(ins);
  } 

  t.minimize();
  fwprintf(ous, L"%d@%d %d %d\n", t.size(), t.numberOfTransitions(), alphabet.size(), patterns.size());

  t.show(alphabet, ous);

  alphabet.write(fst);
  Compression::multibyte_write(patterns.size(), fst);
  for(map<int, Transducer>::iterator it = patterns.begin(); it != patterns.end(); it++) 
  {
    wchar_t buf[50];
    memset(buf, '\0', sizeof(buf));
    swprintf(buf, 50, L"%d", it->first);
    wstring id(buf);
    wcout << id << " " << it->second.size() << endl;
    Compression::wstring_write(id, fst);
    it->second.write(fst);
  } 
  Compression::wstring_write(L"main", fst);
  t.write(fst);
  fclose(fst);

  return 0;
}
