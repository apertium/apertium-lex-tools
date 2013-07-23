#include "../include/irstlm_ranker_frac.h"
using namespace std;

IrstlmRankerFractional::IrstlmRankerFractional(const string &filePath) :
    IrstlmRanker(filePath) {

}

IrstlmRankerFractional::~IrstlmRankerFractional() {

}

int IrstlmRankerFractional::main() {
    int current_line = -1;
    double current_max = -65534.0;
    map<int, double> scores;
    map<int, string> batch;
    int subline = 0;
    int numoptions = 0;
    int maxline = 0;
    double total = 0.0;

    cout.precision(10);
    while (!cin.eof()) {
        string line;
        getline(cin, line);

        if (line.length()>0) {
            // .[][2 0].[]  	Les taches rouges encerclées avec espace Il est ces germes de galaxies .
            string lineno = "";
            int bcc = 0;
            int boc = 0;
            // Yes, this is shitty.
            for(string::iterator x = line.begin(); x != line.end(); x++)
            {
                //cerr << boc << " " << bcc << "; x: " << *x << " lineno:" << lineno << endl ;
                if(*x == '[') boc++;
                if(*x == ']') bcc++;
                if(*x == ' ') break;
                if(boc==2 && bcc <2 && *x != '[')
                {
                    lineno += *x;
                }
            }

            //string lineno = line.substr(line.find("]")+2, line.find(" ")-1);
            string linecon = line.substr(line.find("\t"), line.length());
            int lno = atoi(lineno.c_str());

            if(current_line == -1)
            {
                current_line = lno;
            }
            if(current_line != -1 && current_line != lno)
            {
                for(map<int, string>::iterator it = batch.begin(); it != batch.end(); it++)
                {
                    //cerr << it->first << " " << maxline << " " << scores[it->first] << " " << current_max << endl;
                    if(it->first == maxline)
                    {
                        cout<< fixed << (scores[it->first]/total) << "\t|@|\t" << batch[it->first] <<endl;
                    }
                    else
                    {
                        cout<< fixed <<(scores[it->first]/total) << "\t||\t" << batch[it->first] <<endl;
                    }
                }
                current_max = -65534.0;
                maxline = 0;
                subline = 0;
                total = 0.0;
                numoptions = 0;
                current_line = lno;
                batch.clear();
                scores.clear();
            }

            double pp;
            double log_prob = exp10(score("<s> " + line + " </s>", pp));

            total = total + log_prob;
            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxline = subline;
            }

            batch[subline] = line;
            scores[subline] = log_prob;
            //cout<< log_prob<< "\t||\t" << line <<endl;
            subline = subline + 1;
            numoptions = numoptions + 1;
        }
    }

    for(map<int, string>::iterator it = batch.begin(); it != batch.end(); it++)
    {
//    cerr << it->first << " " << maxline << " " << scores[it->first] << " " << current_max << endl;
        if(it->first == maxline)
        {
            cout<< (scores[it->first]/total) << "\t|@|\t" << batch[it->first] <<endl;
        }
        else
        {
            cout<< (scores[it->first]/total) << "\t||\t" << batch[it->first] <<endl;
        }
    }

    return EXIT_SUCCESS;
}
