#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>
#include <travatar/hyper-graph.h>
#include <travatar/lookup-table-cfglm.h>
#include <travatar/sentence.h>
#include <travatar/input-file-stream.h>
#include <travatar/lm-func.h>
#include <marisa/marisa.h>
#include <boost/foreach.hpp>
#include <sstream>
#include <fstream>

using namespace travatar;
using namespace std;
using namespace boost;

// Based on:
//  A CKY+ Variant for SCFG Decoding Without a Dot Chart
//  Rico Sennrich. SSST 2014.

LookupTableCFGLM::LookupTableCFGLM() : 
      lm_(NULL),
      pop_limit_(-1), trg_factors_(1),
      root_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("S")))),
      unk_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("X")))),
      empty_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("")))) { }

LookupTableCFGLM::~LookupTableCFGLM() {
    BOOST_FOREACH(RuleFSM* rule, rule_fsms_) delete rule;
    if(lm_) delete lm_;  
}

LookupTableCFGLM * LookupTableCFGLM::ReadFromFiles(const std::vector<std::string> & filenames) {
    if(filenames.size() != 1) THROW_ERROR("LookupTableCFGLM currently only supports a single translation model");
    LookupTableCFGLM * ret = new LookupTableCFGLM;
    BOOST_FOREACH(const std::string & filename, filenames) {
        InputFileStream tm_in(filename.c_str());
        cerr << "Reading TM file from "<<filename<<"..." << endl;
        if(!tm_in)
            THROW_ERROR("Could not find TM: " << filename);
        ret->AddRuleFSM(RuleFSM::ReadFromRuleTable(tm_in));
    }
    return ret;
}

void LookupTableCFGLM::LoadLM(const std::string & filename, int pop_limit) {
    if(lm_) THROW_ERROR("Cannot load two LMs for LookupTableCFGLM");
    lm_ = new LMData(filename);
    pop_limit_ = pop_limit;
}

HyperGraph * LookupTableCFGLM::TransformGraph(const HyperGraph & graph) const {

    HyperGraph * ret = new HyperGraph;
    
    Sentence sent = graph.GetWords();
    int N = sent.size();
    vector<CFGChartItem> chart(N*N);
    vector<CFGCollection> collections(N*N);
    CFGPath root;

    for(int i = N-1; i >= 0; i--) {

        // Find single words
        CFGPath next(root, sent, i);
        if(rule_fsms_[0]->GetTrie().predictive_search(next.agent))
            AddToChart(next, sent, N, i, i, false, chart, collections);
        CubePrune(collections[i*N+i]);

        // Find multi-words
        for(int j = i+1; j < N; j++) {
            Consume(root, sent, N, i, i, j-1, chart, collections);
            CubePrune(collections[i*N+j]);
        }

    }

    return ret;
}

void LookupTableCFGLM::Consume(CFGPath & a, const Sentence & sent, int N, int i, int j, int k, vector<CFGChartItem> & chart, vector<CFGCollection> & collections) const {
    bool unary = (i == j);
    if(j == k) {
        CFGPath next(a, sent, j);
        if(rule_fsms_[0]->GetTrie().predictive_search(next.agent))
            AddToChart(next, sent, N, i, k, unary, chart, collections);
    }
    BOOST_FOREACH(const HieroHeadLabels & sym, chart[j*N+k].GetSyms()) {
        CFGPath next(a, sym, j, k);
        if(rule_fsms_[0]->GetTrie().predictive_search(next.agent))
            AddToChart(next, sent, N, i, k, unary, chart, collections);
    }
}

void LookupTableCFGLM::AddToChart(CFGPath & a, const Sentence & sent, int N, int i, int j, bool u, vector<CFGChartItem> & chart, vector<CFGCollection> & collections) const {
    if(!u) {
        if(rule_fsms_[0]->GetTrie().lookup(a.agent))
            collections[i*N+j].AddRules(a, rule_fsms_[0]->GetRules()[a.agent.key().id()]);
    }
    if(rule_fsms_[0]->GetTrie().predictive_search(a.agent))
        for(int k = j+1; j < N; j++)
            Consume(a, sent, N, i, j+1, k, chart, collections);
}

void LookupTableCFGLM::CubePrune(const CFGCollection & collection) const {
    THROW_ERROR("CubePrune not implemented yet");
}
