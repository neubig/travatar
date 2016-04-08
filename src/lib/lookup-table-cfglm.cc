#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>
#include <travatar/hyper-graph.h>
#include <travatar/lookup-table-cfglm.h>
#include <travatar/sentence.h>
#include <travatar/input-file-stream.h>
#include <travatar/lm-func.h>
#include <travatar/vector-hash.h>
#include <travatar/weights.h>
#include <marisa/marisa.h>
#include <boost/foreach.hpp>
#include <sstream>
#include <fstream>
#include <queue>
#include <unordered_set>

using namespace travatar;
using namespace std;
using namespace boost;

// Based on:
//  A CKY+ Variant for SCFG Decoding Without a Dot Chart
//  Rico Sennrich. SSST 2014.

CFGChartItem::~CFGChartItem() {
    BOOST_FOREACH(StatefulNodeMap::value_type & val, nodes_)
        BOOST_FOREACH(StatefulNode* ptr, val.second)
            delete ptr;
}


Real CFGChartItem::GetHypScore(const HieroHeadLabels & label, int pos) const {
    //cerr << " populated_ == " << populated_ << " (" << (long)&populated_ << ")" << endl;
    assert(populated_);
    StatefulNodeMap::const_iterator it = nodes_.find(label);
    assert(it != nodes_.end());
    Real ret = (it->second.size() > pos) ? it->second[pos]->first->CalcViterbiScore() : -REAL_MAX;
    //cerr << " label: " << label << ", pos: " << pos << ", it->second.size(): " << it->second.size() << ", more_than: " << (it->second.size() > pos) << ", ret: " << ret << endl;
    return ret;
}


void CFGChartItem::AddStatefulNode(const HieroHeadLabels & label, HyperNode* node, const std::vector<lm::ngram::ChartState> & state) {
    nodes_[label].push_back(new StatefulNode(node, state));
}

const CFGChartItem::StatefulNode & CFGChartItem::GetStatefulNode(const HieroHeadLabels & label, int pos) const {
    assert(populated_);
    StatefulNodeMap::const_iterator it = nodes_.find(label);
    //cerr << " pos==" << pos << ", it->second.size()==" << it->second.size() << endl;
    assert(it != nodes_.end() && it->second.size() > pos);
    return *it->second[pos];
}

class StatefulNodeScoresMore {
public:
    bool operator()(const CFGChartItem::StatefulNode* x, const CFGChartItem::StatefulNode* y) {
        return x->first->GetViterbiScore() > y->first->GetViterbiScore();
    }
};

void CFGChartItem::FinalizeNodes() {
    BOOST_FOREACH(StatefulNodeMap::value_type & node_set, nodes_) {
        if(node_set.second.size() > 1)
            sort(node_set.second.begin(), node_set.second.end(), StatefulNodeScoresMore());
    }
    //cerr << " populated_ == " << populated_ << " (" << (long)&populated_ << ")" << endl;
    populated_ = true;
}

LookupTableCFGLM::LookupTableCFGLM() : 
      pop_limit_(-1), chart_limit_(-1), trg_factors_(1),
      root_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("S")))),
      unk_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("X")))),
      empty_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("")))),
      weights_(new Weights) { }

LookupTableCFGLM::~LookupTableCFGLM() {
    BOOST_FOREACH(RuleFSM* rule, rule_fsms_) delete rule;
    BOOST_FOREACH(LMData * lm, lm_data_) delete lm;
}

LookupTableCFGLM * LookupTableCFGLM::ReadFromFiles(const std::vector<std::string> & filenames) {
    if(filenames.size() != 1) THROW_ERROR("LookupTableCFGLM currently only supports a single translation model");
    LookupTableCFGLM * ret = new LookupTableCFGLM;
    BOOST_FOREACH(const std::string & filename, filenames) {
        InputFileStream tm_in(filename.c_str());
        //cerr << "Reading TM file from "<<filename<<"..." << endl;
        if(!tm_in)
            THROW_ERROR("Could not find TM: " << filename);
        ret->AddRuleFSM(RuleFSM::ReadFromRuleTable(tm_in));
    }
    return ret;
}

void LookupTableCFGLM::LoadLM(const std::string & filename) {
    lm_data_.push_back(new LMData(filename));
    funcs_.push_back(LMFunc::CreateFromType(lm_data_[lm_data_.size()-1]->GetType())); 
}

bool LookupTableCFGLM::PredictiveSearch(marisa::Agent & agent) const {
    BOOST_FOREACH(const RuleFSM * fsm, rule_fsms_) {
        marisa::Agent tmp_agent; tmp_agent.set_query(agent.query().ptr(), agent.query().length());
        if(fsm->GetTrie().predictive_search(tmp_agent))
            return true;
    }
    return false;
}

void LookupTableCFGLM::Consume(CFGPath & a, const Sentence & sent, int N, int i, int j, int k, vector<CFGChartItem> & chart, vector<CFGCollection> & collections) const {
    //cerr << "Consume(" << CFGPath::PrintAgent(a.agent) << " len==" << a.agent.query().length() << ", " << sent << ", " << N << ", " << i << ", " << j << ", " << k << ")" << endl;
    bool unary = (i == j);
    if(j == k) {
        CFGPath next(a, sent, j);
        if(PredictiveSearch(next.agent))
            AddToChart(next, sent, N, i, k, unary, chart, collections);
    }
    BOOST_FOREACH(const CFGChartItem::StatefulNodeMap::value_type & sym, chart[j*N+k].GetNodes()) {
        CFGPath next(a, sym.first, j, k);
        if(PredictiveSearch(next.agent))
            AddToChart(next, sent, N, i, k, unary, chart, collections);
    }
}

void LookupTableCFGLM::AddToChart(CFGPath & a, const Sentence & sent, int N, int i, int j, bool u, vector<CFGChartItem> & chart, vector<CFGCollection> & collections) const {
    //cerr << "AddToChart(" << CFGPath::PrintAgent(a.agent) << " len==" << a.agent.query().length() << ", " << sent << ", " << N << ", " << i << ", " << j << ", " << u << ")" << endl;
    if(!u) {
        BOOST_FOREACH(const RuleFSM * fsm, rule_fsms_) {
            if(fsm->GetTrie().lookup(a.agent)) {
                //cerr << " Looking up and found rules" << endl;
                collections[i*N+j].AddRules(a, fsm->GetRules()[a.agent.key().id()]);
            } else {
                //cerr << " Looking up and didn't find rules" << endl;
            }
        }
    }
    if(PredictiveSearch(a.agent))
        for(int k = j+1; k < N; k++)
            Consume(a, sent, N, i, j+1, k, chart, collections);
}

void LookupTableCFGLM::CubePrune(int N, int i, int j, vector<CFGCollection> & collection, vector<CFGChartItem> & chart, HyperGraph & ret) const {
    //cerr << "CubePrune(" << N << ", " << i << ", " << j << ")" << endl;
    // Don't build already finished charts
    int id = i*N + j;
    assert(!chart[id].IsPopulated());
    // The priority queue of values yet to be expanded
    priority_queue<pair<Real, vector<int> > > hypo_queue;
    // The set indicating already-expanded combinations
    unordered_set<vector<int>, VectorHash<int> > finished;
    // For each rule in the collection, add its best edge to the chart
    const RuleVec & rules = collection[i*N+j].GetRules();
    const CFGCollection::SpanVec & spans = collection[i*N+j].GetSpans();
    const CFGCollection::LabelVec & labels = collection[i*N+j].GetLabels();
    assert(rules.size() == spans.size());

    // Score the top hypotheses of each rule
    vector<Real> rule_scores(rules.size());
    //cerr << " Scoring hypotheses for " << rules.size() << " rules" << endl;
    for(size_t i = 0; i < rules.size(); i++) {
        // Get the base score for the rule
        Real score = weights_->GetCurrent() * rules[i]->GetFeatures();
        const vector<pair<int,int> > & path = *spans[i];
        const vector<HieroHeadLabels> & lab = *labels[i];
        assert(i < labels.size());
        //cerr << " Lab: " << lab.size() << ", path: " << path.size() << ", rule_score: " << score << endl;
        assert(lab.size() == path.size());
        for(size_t j = 0; j < path.size() && score != -REAL_MAX; j++) {
            int id = path[j].first * N + path[j].second;
            score += chart[id].GetHypScore(lab[j], 0);
        }
        //cerr << " score = " << score << endl;
        if(score != -REAL_MAX) {
            vector<int> pos(spans[i]->size()+1,0); pos[0] = i;
            hypo_queue.push(make_pair(score, pos));
        }
    }
    
    // Create a map for recombination
    typedef std::pair<HieroHeadLabels, std::vector<lm::ngram::ChartState> > RecombIndex;
    typedef std::map<RecombIndex, HyperNode*> RecombMap;
    RecombMap recomb_map;

    // Go through the priority queue
    for(int num_popped = 0; hypo_queue.size() != 0 && 
                            (pop_limit_ < 0 || num_popped < pop_limit_) &&
                            (chart_limit_ < 0 || recomb_map.size() < chart_limit_); num_popped++) {
        // Pop the top hypothesis
        Real top_score = hypo_queue.top().first;
        vector<int> id_str = hypo_queue.top().second;
        hypo_queue.pop();
        const vector<pair<int,int> > & path = *spans[id_str[0]];
        const TranslationRuleHiero & rule = *rules[id_str[0]];
        // Create the next edge
        HyperEdge * next_edge = new HyperEdge;
        next_edge->SetFeatures(rule.GetFeatures());
        next_edge->SetTrgData(rule.GetTrgData());
        // next_edge->SetSrcStr(rule.GetSrcStr());
        vector<lm::ngram::ChartState> my_state(lm_data_.size());
        vector<vector<lm::ngram::ChartState> > states(path.size());
        for(size_t j = 0; j < path.size(); j++) {
            const pair<int,int> & my_span = (*spans[id_str[0]])[j];
            const CFGChartItem::StatefulNode & node = chart[my_span.first*N + my_span.second].GetStatefulNode((*labels[id_str[0]])[j], id_str[j+1]);
            //cerr << " Adding tail: " << node.first->GetId() << endl;
            next_edge->AddTail(node.first);
            states[j] = node.second;
        }
        // Calculate the language model score
        Real total_score = 0;
        vector<SparsePair> lm_features;
        for(int lm_id = 0; lm_id < (int)lm_data_.size(); lm_id++) {
            LMData* data = lm_data_[lm_id];
            pair<Real,int> lm_scores = funcs_[lm_id]->CalcNontermScore(data, next_edge->GetTrgData()[data->GetFactor()].words, states, lm_id, my_state[lm_id]);
            // Add to the features and the score
            total_score += lm_scores.first * data->GetWeight() + lm_scores.second * data->GetUnkWeight();
            if(lm_scores.first != 0.0)
                lm_features.push_back(make_pair(data->GetFeatureName(), lm_scores.first));
            if(lm_scores.second != 0)
                lm_features.push_back(make_pair(data->GetUnkFeatureName(), lm_scores.second));
        }
        next_edge->GetFeatures() += SparseVector(lm_features);
        // Add the hypothesis to the hypergraph
        RecombIndex ridx = make_pair(rule.GetHeadLabels(), my_state);
        RecombMap::iterator rit = recomb_map.find(ridx);
        ret.AddEdge(next_edge);
        if(rit != recomb_map.end()) {
            rit->second->AddEdge(next_edge);
            next_edge->SetHead(rit->second);
        } else {
            HyperNode * node = new HyperNode;
            node->SetSpan(make_pair(i, j+1));
            next_edge->SetHead(node);
            ret.AddNode(node);
            chart[id].AddStatefulNode(rule.GetHeadLabels(), node, my_state);
            node->AddEdge(next_edge);
            recomb_map.insert(make_pair(ridx, node));
        }
        //cerr << " hypo_queue.size(): " << hypo_queue.size() << endl;
        // Advance the hypotheses
        for(size_t j = 0; j < path.size(); j++) {
            const pair<int,int> & my_span = (*spans[id_str[0]])[j];
            Real my_score = top_score - chart[my_span.first*N + my_span.second].GetHypScoreDiff((*labels[id_str[0]])[j], id_str[j+1]);
            if(my_score > -REAL_MAX/2) {
                //cerr << " adding hypothesis with score: " << my_score << endl;
                vector<int> pos(id_str); pos[j+1]++;
                hypo_queue.push(make_pair(my_score, pos));
            }
        } 
    }

    // Sort the nodes in each bin, and mark the chart populated
    //cerr << " Finalizing " << id << endl;
    chart[id].FinalizeNodes();

}

HyperGraph * LookupTableCFGLM::TransformGraph(const HyperGraph & graph) const {

    HyperGraph * ret = new HyperGraph;
    ret->SetWords(graph.GetWords());
       
    Sentence sent = graph.GetWords();
    int N = sent.size();
    vector<CFGChartItem> chart(N*N);
    vector<CFGCollection> collections(N*N);
    CFGPath root_path;

    // Add the root note
    HyperNode * root = new HyperNode;
    root->SetSpan(make_pair(0, N));
    ret->AddNode(root);

    for(int i = N-1; i >= 0; i--) {
        // Find single words
        CFGPath next(root_path, sent, i);
        if(PredictiveSearch(next.agent))
            AddToChart(next, sent, N, i, i, false, chart, collections);
        CubePrune(N, i, i, collections, chart, *ret);

        // Find multi-words
        for(int j = i+1; j < N; j++) {
            Consume(root_path, sent, N, i, i, j, chart, collections);
            CubePrune(N, i, j, collections, chart, *ret);
        }
    }

    // Build the final nodes
    BOOST_FOREACH(const CFGChartItem::StatefulNodeMap::value_type snm, chart[N-1].GetNodes()) {
        BOOST_FOREACH(const CFGChartItem::StatefulNode * sn, snm.second) {
            HyperEdge * edge = new HyperEdge(root);
            edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors, CfgData(Sentence(1, -1))));
            //cerr << " Adding tail: " << sn->first->GetId() << endl;
            edge->AddTail(sn->first);
            Real total_score = 0;
            for(int lm_id = 0; lm_id < (int)lm_data_.size(); lm_id++) {
                LMData* data = lm_data_[lm_id];
                Real my_score = funcs_[lm_id]->CalcFinalScore(data->GetLM(), sn->second[lm_id]);
                if(my_score != 0.0)
                    edge->GetFeatures().Add(data->GetFeatureName(), my_score);
                total_score += my_score * data->GetWeight();
            }
            edge->SetScore(total_score);
            ret->AddEdge(edge);
            root->AddEdge(edge);
        }
    }
    root->CalcViterbiScore();

    return ret;
}
