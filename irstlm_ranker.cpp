#include "irstlm_ranker.h"
using namespace std;

IrstlmRanker::IrstlmRanker(const string &filePath, vector<double> params) {
    bool val = this->load(filePath, 1.0);

    if(!val) {
        cerr<<"There was a problem when loadling the language model from file '";
        cerr << filePath <<"'"<<endl;
        exit(EXIT_FAILURE);
    }
	this->lowerBound = params[0];
	this->upperBound = params[1];
	this->filter = params[2];

    cout.precision(10);
    wcout.precision(10);

	lineno = 0;
	sublineno = 0;
	maxlineno = 0;
    current_line = -1;
    current_max = -65534.0;
}

IrstlmRanker::~IrstlmRanker() {

}


void IrstlmRanker::reset() {
    current_max = -65534.0;
    maxlineno = 0;
    sublineno = 0;
    current_line = lineno;
    batch.clear();
    scores.clear();
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

void IrstlmRanker::printScores(vector<string> batch, vector<double> scores, double total)
{
    vector<string>::iterator it = batch.begin();
    for(int i = 0; i < batch.size(); i++)
    {
        double score = scores[i] / total;
        string line = batch[i];
		bool outside = this->filter && 
			(score > this->upperBound || score < this->lowerBound);
		bool inside = this->filter && !outside;

        cout << fixed << score << "\t|";
		if (outside) {
			cout << "-|\t|";
		} else if (inside) {
			cout << "+|\t|";
		}
        if(i == maxlineno) {
           cout << "@";
        }
        cout << "|\t" << line << endl;
    }
}


int IrstlmRanker::standard() {
	maxline = -1;
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if (line.length()>0) {
            double pp;
            double log_prob = score(line, pp);
            batch.push_back(line);
			scores.push_back(log_prob);
        }
    }
	printScores(batch, scores, 1.0);
    return EXIT_SUCCESS;
}


int IrstlmRanker::fractional() {
	cout.precision(10);
	double total = 0.0;
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if (line.length()>0) {
            vector<string> tokens = parseLine(line);
            lineno = atoi(tokens[0].c_str());
            if(current_line == -1)
            {
                current_line = lineno;
            }
            if(current_line != lineno)
            {
                printScores(batch, scores, total);
                reset();
				total = 0.0;
            }

            double pp;
            double log_prob = exp10(score(line, pp));

            total = total + log_prob;
            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxlineno = sublineno;
            }

			batch.push_back(line);
            scores.push_back(log_prob);
			sublineno ++;
        }
    }
    printScores(batch, scores, total);

    return EXIT_SUCCESS;
}

int IrstlmRanker::max() {
    double total = 1.0;
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if (line.length()>0) {
            vector<string> tokens = parseLine(line);
            lineno = atoi(tokens[0].c_str());
            if(current_line == -1)
            {
                current_line = lineno;
            }
            if(current_line != lineno)
            {
                printScores(batch, scores, total);
                reset();
            }
            double pp;
            double log_prob = score(line, pp);

            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxlineno = sublineno;
            }

            batch.push_back(line);
            scores.push_back(log_prob);
			sublineno ++;
        }
    }
    printScores(batch, scores, total);
    return EXIT_SUCCESS;
}

int IrstlmRanker::totals() {
    double total = 0.0;
    int numlines = 0;
	maxlineno = -1;
    while (!cin.eof()) {
        string line;
        getline(cin, line);

        if (line.length()>0) {
            double pp;
            double log_prob = score(line, pp);
			batch.push_back(line);
			scores.push_back(log_prob);
            total += log_prob ;
            numlines++;
        }
    }
	printScores(batch, scores, 1.0);
    cout << "log_total: " << total << endl;
    cout << "prob_total: " << exp10(total) << endl;
    cout << "log_avg: " << total/numlines << endl;
    cout << "prob_avg: " << exp10(total)/numlines << endl;

    return EXIT_SUCCESS;
}
// === MAIN === //

vector<double> parseArgs(int argc, char **argv) {

	vector<double> params;

	params.push_back(std::numeric_limits<double>::min()); 
	params.push_back(std::numeric_limits<double>::max());
	params.push_back(0);


	for(int i = 3; i < argc; i++) {
		if(strcmp(argv[i], "-l") == 0) {
			params[0] = atof(argv[i+1]);
			params[2] = 1;
			i++;
		} if(strcmp(argv[i], "-u") == 0) {
			params[1] = atof(argv[i+1]);
			params[2] = 1;
			i++;
		} 
	}
	cerr << "ASD"<< endl;
	return params;
	
}

void printError(char* name) {
    wcerr<<"Error: Wrong number of parameters"<<endl;
    wcerr<<"Usage: "<<name<<" lm_file mode [lower bound] [upper bound]"<<endl;
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

    if (argc < 3 && argc > 7) {
        printError(argv[0]);
    }
	
	vector<double> params = parseArgs(argc, argv);

    IrstlmRanker irstlm_ranker(argv[1], params);
    string mode(argv[2]);
    if (mode == "--standard" || mode == "-s") {
        irstlm_ranker.standard();
    } else if (mode == "--fractional-counts" || mode == "-f") {
        irstlm_ranker.fractional();
    } else if (mode == "--total-counts" || mode == "-t") {
        irstlm_ranker.totals();
    } else if (mode == "--max-count" || mode == "-m") {
        irstlm_ranker.max();
    } else {
        printError(argv[0]);
    }

    return 0;
}

