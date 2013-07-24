#include "irstlm_ranker.h"
using namespace std;

IrstlmRanker::IrstlmRanker(const string &filePath) {
    bool val = this->load(filePath, 1.0);

    if(!val) {
        cerr<<"There was a problem when loadling the language model from file '";
        cerr << filePath <<"'"<<endl;
        exit(EXIT_FAILURE);
    }

    cout.precision(10);
    wcout.precision(10);

    current_line = "-1";
    current_max = -65534.0;
    maxlineno = "0";
    total = 0.0;

}

IrstlmRanker::~IrstlmRanker() {

}

double IrstlmRanker::score(const string &frame, double &pp) {
    string buf;

    vector<string> s_unigrams;
    deque<string> buffer;

    stringstream ss(frame);
    ngram* m_lmtb_ng;
    int lmId = 0;
    float prob = 0, sprob = 0;

    m_lmtb_ng = new ngram(m_lmtb->getDict()); // ngram of words
    m_lmtb_ng->size = 0;

    int count = 0;
//	s_unigrams.push_back("<s>");
    while (ss >> buf) {
        if(count == 1) {
            s_unigrams.push_back(trim(buf));
        }
        if(strstr(trim(buf).c_str(),"].[]")) {
            count = 1;
        }
    }
//	s_unigrams.push_back("</s>");
    // It is assumed that sentences start with <s>
    buffer.push_back(s_unigrams.at(0));

    for (unsigned i = 1; i < s_unigrams.size(); i++) {
        buffer.push_back(s_unigrams.at(i));
        if (buffer.size() > m_nGramOrder)
            buffer.pop_front();

        string buffer_str="";
        m_lmtb_ng = new ngram(m_lmtb->getDict()); // ngram of words
        for (unsigned j = 0; j < buffer.size(); j++) {
            if (j>0) buffer_str += " ";
            buffer_str += buffer[j];
            lmId = m_lmtb->getDict()->encode(buffer[j].c_str());
            m_lmtb_ng->pushc(lmId);
        }

        prob = m_lmtb->clprob(*m_lmtb_ng);
        delete m_lmtb_ng;

        sprob += prob;
        cerr << "_" << m_nGramOrder << ": " << buffer_str << " " << prob << endl;
    }

    //Perplexity
    pp = exp((-sprob * log(10.0))/(s_unigrams.size()-1));  //Do not take into account <s>, but </s>

    return sprob;
}
string IrstlmRanker::trim(const string& o) {
    string ret = o;
    const char* chars = "\n\t\v\f\r *"; // whitespace and unknown words
    ret.erase(ret.find_last_not_of(chars)+1);
    ret.erase(0, ret.find_first_not_of(chars));
    return ret;
}

bool IrstlmRanker::load(const string &filePath, float weight) {
    m_weight  = weight;

    m_filePath = filePath;

    // Open the input file (possibly gzipped)
    std::streambuf *m_streambuf;
    std::filebuf* fb = new std::filebuf();
    fb->open(filePath.c_str(), std::ios::in);
    m_streambuf = fb;
    std::istream inp(m_streambuf);

    // case (standard) LMfile only: create an object of lmtable
    std::string infile(filePath);
    m_lmtb=NULL;
    m_lmtb = m_lmtb->CreateLanguageModel(infile, 0, 0);
    m_lmtb->load(infile);
    m_lmtb_size = m_lmtb->maxlevel();
    m_nGramOrder = m_lmtb->maxlevel();

    // LM can be ok, just outputs warnings
    m_unknownId = m_lmtb->getDict()->oovcode(); // at the level of micro tags
    cerr<<"IRST: m_unknownId="<<m_unknownId<<endl;

    if (m_lmtb_dub >0) {
        m_lmtb->setlogOOVpenalty(m_lmtb_dub);
    }

    return true;
}

int IrstlmRanker::standard() {
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if (line.length()>0) {
            double pp;
            double log_prob = score(line, pp);
            cout<< log_prob<< "\t||\t" << line <<endl;
        }
    }
    return EXIT_SUCCESS;
}

void IrstlmRanker::printScores(map<string, string> batch, map<string, double> scores)
{
    map<string, string>::iterator it = batch.begin();
    for(; it != batch.end(); it++)
    {
        double score = scores[it->first];
        string line = batch[it->first];
        cout << fixed << score << "\t|";
        if(it->first == maxlineno) {
            cout << "@";
        }
        cout << "|\t" << line << endl;
    }
}

int IrstlmRanker::fractional() {
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if (line.length()>0) {
            vector<string> tokens = parseLine(line);
            lineno = tokens[0];
            sublineno = tokens[1];
            if(current_line == "-1")
            {
                current_line = lineno;
            }
            if(current_line != lineno)
            {
                printScores(batch, scores);
                reset();
            }

            double pp;
            double log_prob = exp10(score(line, pp));

//			cout << score("<s> " + line + " </s>", pp) << endl;
//			cout << exp10(score("<s> " + line + " </s>", pp)) << endl;

            total = total + log_prob;
            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxlineno = sublineno;
            }

            batch[sublineno] = line;
            scores[sublineno] = log_prob;
        }
    }
    printScores(batch, scores);

    return EXIT_SUCCESS;
}

vector<string> IrstlmRanker::parseLine(string line) {
    string buf;
    stringstream ss(line);
    vector<string> tokens;
    int count = 0;
    while (ss >> buf) {
        if (count == 0) {
            buf = buf.substr(4, buf.length());
        } else if (count == 1) {
            buf = buf.substr(0, buf.length()-4);
        }
        tokens.push_back(buf);
        count++;
    }
    return tokens;
}

void IrstlmRanker::reset() {
    current_max = -65534.0;
    maxlineno = "0";
    sublineno = "0";
    current_line = lineno;
    batch.clear();
    scores.clear();
}

int IrstlmRanker::max() {
    total = 1.0;
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if (line.length()>0) {
            vector<string> tokens = parseLine(line);
            lineno = tokens[0];
            sublineno = tokens[1];
            if(current_line == "-1")
            {
                current_line = lineno;
            }
            if(current_line != lineno)
            {
                printScores(batch, scores);
                reset();
            }
            double pp;
            double log_prob = score(line, pp);

            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxlineno = sublineno;
            }

            batch[sublineno] = line;
            scores[sublineno] = log_prob;
        }
    }
    printScores(batch, scores);
    return EXIT_SUCCESS;
}

int IrstlmRanker::totals() {
    double total = 0.0;
    int numlines = 0;
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        cerr << "@: " << line << endl;
        if (line.length()>0) {
            double pp;
            double log_prob = score(line, pp);
            cout<< log_prob<< "\t||\t" << line <<endl;

            total += log_prob ;
            numlines++;
        }
    }
    cout << "log_total: " << total << endl;
    cout << "prob_total: " << exp10(total) << endl;
    cout << "log_avg: " << total/numlines << endl;
    cout << "prob_avg: " << exp10(total)/numlines << endl;

    return EXIT_SUCCESS;
}


void printError(char* name) {
    wcerr<<"Error: Wrong number of parameters"<<endl;
    wcerr<<"Usage: "<<name<<" lm_file mode"<<endl;
    wcerr<<"modes:" << endl;
    wcerr<<"\t -s | --standard"<<endl;
    wcerr<<"\t -f | --fractional-counts"<<endl;
    wcerr<<"\t -t | --total-counts"<<endl;
    wcerr<<"\t -m | --max-count"<<endl;
    exit(EXIT_FAILURE);
}
int main(int argc, char ** argv) {

// Is this really necessary?
// I don't know :)
    if(setlocale(LC_CTYPE, "") == NULL) {
        wcerr << L"Warning: unsupported locale, fallback to \"C\"" << endl;
        setlocale(LC_ALL, "C");
    }

    if (argc != 3) {
        printError(argv[0]);
    }
    IrstlmRanker irstlm_ranker(argv[1]);
    string mode(argv[2]);
    if (mode == "--standard" || mode == "-s") {
        irstlm_ranker.standard();
    } else if (mode == "--fractional-counts" || mode == "-f") {
        irstlm_ranker.fractional();
        //irstlm_ranker = new IrstlmRankerFractional(argv[1]);
    } else if (mode == "--total-counts" || mode == "-t") {
        irstlm_ranker.totals();
    } else if (mode == "--max-count" || mode == "-m") {
        irstlm_ranker.max();
    } else {
        printError(argv[0]);
    }

    return 0;
}

