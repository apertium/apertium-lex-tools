#include "../include/irstlm_ranker.h"
using namespace std;

IrstlmRanker::IrstlmRanker(const string &filePath) {
	bool val = this->load(filePath, 1.0);
	
  if(!val) {
    cerr<<"There was a problem when loadling the language model from file '"<< filePath <<"'"<<endl;
    exit(EXIT_FAILURE);
  }
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
    while (ss >> buf) {
        if(count == 1) {
            s_unigrams.push_back(trim(buf));
        }
        if(strstr(trim(buf).c_str(),"].[]")) {
            count = 1;
        }
    }
	
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

    if (m_lmtb_dub >0) m_lmtb->setlogOOVpenalty(m_lmtb_dub);

    return true;
}
