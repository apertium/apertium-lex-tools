#include "irstlm_ranker.h"
using namespace std;

IrstlmRanker::IrstlmRanker(const string &filePath, 
						   char *tmtrans_path, 
						   vector<double> params) 
{
    bool val = this->load(filePath, 1.0);

    if(!val) {
        cerr<<"There was a problem when loadling the language model from file '";
        cerr << filePath <<"'"<<endl;
        exit(EXIT_FAILURE);
    }
	this->probMassThr = params[0];
	tmtrans.open(tmtrans_path);
	if (!tmtrans.is_open()) {
		cout << "Could not open trimmed multitrans file: " << tmtrans_path << endl;
		exit(-1);
	}
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
    probs.clear();
	logScores.clear();
	sortedIndex.clear();
	norm = 0;
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

    int count = 0;
	s_unigrams.push_back("<s>");
    while (ss >> buf) {
        if(count == 1) {
            s_unigrams.push_back(trim(buf));
        }
        if(strstr(trim(buf).c_str(),"].[]")) {
            count = 1;
        }
    }
	s_unigrams.push_back("</s>");
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

void IrstlmRanker::normalizeProbabilities() {
	
	for(int i = 0; i < probs.size(); i++) {
		probs[i] = probs[i] / this->norm;
		if (probs[i] != probs[i]) {
			probs[i] = 0;
		}
	}
}

void IrstlmRanker::printScores(vector<long double> scores)
{	
	
	double thr = 0.5;
	vector<double> thresholds;
	set<int> positiveIndex;
	long double probSum = 0.0;

	for(int i = 0; i < sortedIndex.size(); i++) {
		int idx = sortedIndex[i];
		probSum += probs[idx];
		if (probSum > thr) {
			thr += 0.1;
			if (i != 0) {
				while(probSum > thr) {
					thresholds.push_back(-1);
					thr += 0.1;
				}
			}
			thresholds.push_back(i);
		}
	}
	
	if (thresholds.size() == 0) {
		for(int i = 0; i < 6; i++) {
			thresholds.push_back(-1);
		}
	}

    for(int i = 0; i < batch.size(); i++)
    {
        long double score = scores[i];
        string line = batch[i];

		double c = 5;
		int rank = find(sortedIndex.begin(), sortedIndex.end(), i) - sortedIndex.begin();
		for(int j = 0; j < thresholds.size(); j++) {
			if (rank <= thresholds[j]) {
				c = j;
				break;
			}
		}
		bool inside = false;
		double thr_idx = probMassThr * 10.0 - 5;
		if (rank <= thresholds[ceil(thr_idx)]) {
			inside = true;
		}
		cout << line;
        cout << "\t" << score;
		if(i == maxlineno) {
           cout << "\t|@|\t";
		} else if (inside) {
			cout << "\t|+|\t";
		} else {
			cout << "\t|-|\t";
		}
		cout << "|" << (c + 5) / 10 << "|\t" << rank << endl;
    }
}

int IrstlmRanker::fractional() {
		cout.precision(10);
		while (!cin.eof()) {
        string line;
        getline(cin, line);

		string tt_line;
		getline(this->tmtrans, tt_line);

        if (line.length() > 0) {
            vector<string> tokens = parseLine(line);
            lineno = atoi(tokens[0].c_str());
			int current_sublineno = atoi(tokens[1].c_str());
            
            if(current_sublineno < sublineno)
            {
				normalizeProbabilities();
                printScores(probs);
			
                reset();
            }

            double pp;
            long double log_prob = score(line, pp);
			long double prob = exp10(log_prob);

            this->norm += prob;
            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxlineno = sublineno;
            }

			batch.push_back(tt_line);
            probs.push_back(prob);
			logScores.push_back(log_prob);
			insertSortedIndex(prob);
			sublineno ++;
        }
    }
	
	normalizeProbabilities();
    printScores(probs);

    return EXIT_SUCCESS;
}

int IrstlmRanker::standard() {
    while (!cin.eof()) {
        string line;
        getline(cin, line);

		string tt_line;
		getline(this->tmtrans, tt_line);
        if (line.length()>0) {
            vector<string> tokens = parseLine(line);
            lineno = atoi(tokens[0].c_str());
            if(current_line == -1)
            {
                current_line = lineno;
            }
            if(current_line != lineno)
            {
				normalizeProbabilities();
                printScores(logScores);
                reset();
            }
            double pp;
            double log_prob = score(line, pp);
			double prob = exp10(log_prob);
			if (isnan(prob)) {
				prob = 0;
			}
			this->norm += prob;
            if(log_prob > current_max)
            {
                current_max = log_prob;
                maxlineno = sublineno;
            }
			
            batch.push_back(tt_line);
            probs.push_back(prob);
			logScores.push_back(log_prob);
			insertSortedIndex(prob);
			sublineno ++;
        }
    }	
	normalizeProbabilities();
    printScores(logScores);
    return EXIT_SUCCESS;
}

void IrstlmRanker::insertSortedIndex(long double prob) {
	if (sortedIndex.size() == 0) {
		sortedIndex.push_back(sublineno);
		return;
	}
	vector<int>::iterator it = sortedIndex.begin();
	int lowBound = 0;
	int highBound = sortedIndex.size() - 1;
	if (prob <= probs[sortedIndex[highBound]]) {
 		sortedIndex.push_back(sublineno);
		return;
	} else if (prob >= probs[sortedIndex[lowBound]]) {
		sortedIndex.insert(it, sublineno);
		return;
	}

	while(true) {
		int midIdx = (lowBound + highBound ) / 2;
		if (lowBound == highBound) {
			sortedIndex.insert(it + lowBound, sublineno);
			break;
		} else if (lowBound == highBound - 1) {
			sortedIndex.insert(it + lowBound + 1, sublineno);
			break;
		}
		
		if (prob > probs[sortedIndex[midIdx]]) {
			highBound = midIdx;
		} else {
			lowBound = midIdx;
		}
	}
}


// === MAIN === //

vector<double> parseArgs(int argc, char **argv) {

	vector<double> params;

	params.push_back(1); 


	for(int i = 3; i < argc; i++) {
		if(strcmp(argv[i], "-m") == 0  ||
		   strcmp(argv[i], "--probability-mass-kept") == 0) {
			params[0] = atof(argv[i+1]);
			i++;
		}
	}
	return params;
}

void printError(char* name) {
    cout<<"Error: Wrong number of parameters"<<endl;
    cout<<"Usage: "<<name<<" <lm_file> <trimmed-multitrans-file> <mode> [-m | --probability-mass-threshold]"<<endl;
    cout<<"modes:" << endl;
    cout<<"\t -s | --standard"<<endl;
    cout<<"\t -f | --fractional-counts"<<endl;
    exit(EXIT_FAILURE);
}



int main(int argc, char ** argv) {

	// Is this really necessary?
	// I don't know :)

    if(setlocale(LC_CTYPE, "") == NULL) {
        wcerr << L"Warning: unsupported locale, fallback to \"C\"" << endl;
        setlocale(LC_ALL, "C");
    }

    if (argc != 4 && argc != 6) {
        printError(argv[0]);
		exit(1);
    }
	
	vector<double> params = parseArgs(argc, argv);
    IrstlmRanker irstlm_ranker(argv[1], argv[2], params);

    string mode(argv[3]);
    if (mode == "--standard" || mode == "-s") {
        irstlm_ranker.standard();
    } else if (mode == "--fractional-counts" || mode == "-f") {
        irstlm_ranker.fractional();
    } else {
        printError(argv[0]);
    }

    return 0;
}

