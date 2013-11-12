#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <lm/left.hh>
#include <vector>
#include <queue>
#include <map>
#include <travatar/lm-composer-bu.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace lm::ngram;

namespace travatar {

class NodeScoreMore {
public:
    bool operator()(const HyperNode* x, const HyperNode* y) {
        return x->GetViterbiScore() > y->GetViterbiScore();
    }
};

class NodeSrcLess {
public:
    bool operator()(const HyperNode* x, const HyperNode* y) {
        return x->GetSpan().first < y->GetSpan().second;
    }
};

}

const ChartEntry & LMComposerBU::BuildChart(
                    const HyperGraph & parse,
                    vector<shared_ptr<ChartEntry> > & chart, 
                    vector<ChartState> & states, 
                    int id,
                    HyperGraph & rule_graph) const {
    // Save the nodes for easy access
    const vector<HyperNode*> & nodes = parse.GetNodes();
    // Don't build already finished charts
    if(chart[id].get() != NULL) return *chart[id];
    // cerr << "Building chart @ " << id << endl;
    chart[id].reset(new ChartEntry);
    // The priority queue of values yet to be expanded
    priority_queue<pair<double, GenericString<int> > > hypo_queue;
    // The hypothesis combination map
    map<ChartState, HyperNode*> hypo_comb;
    // The set indicating already-expanded combinations
    unordered_set<GenericString<int>, GenericHash<GenericString<int> > > finished;
    // For each edge outgoing from this node, add its best hypothesis
    // to the chart
    const vector<HyperEdge*> & node_edges = nodes[id]->GetEdges();
    for(int i = 0; i < (int)node_edges.size(); i++) {
        HyperEdge * my_edge = node_edges[i];
        GenericString<int> q_id(my_edge->GetTails().size()+1);
        q_id[0] = i;
        double viterbi_score = my_edge->GetScore();
        for(int j = 1; j < (int)q_id.length(); j++) {
            q_id[j] = 0;
            const ChartEntry & my_entry = BuildChart(parse, chart, states, my_edge->GetTail(j-1)->GetId(), rule_graph);
            viterbi_score += my_entry[0]->CalcViterbiScore();
        }
        hypo_queue.push(make_pair(viterbi_score, q_id));
    }
    // For each edge on the queue, process it
    int num_popped = 0;
    WordId feature_id = Dict::WID(feature_name_);
    while(hypo_queue.size() != 0) {
        if(num_popped++ >= stack_pop_limit_) break;
        // Get the score, id string, and edge
        double top_score = hypo_queue.top().first;
        GenericString<int> id_str = hypo_queue.top().second;
        const HyperEdge * id_edge = nodes[id]->GetEdge(id_str[0]);
        hypo_queue.pop();
        // Find the chart state and LM probability
        HyperEdge * next_edge = new HyperEdge;
        next_edge->SetFeatures(id_edge->GetFeatures());
        next_edge->SetTrgWords(id_edge->GetTrgWords());
        next_edge->SetRuleStr(id_edge->GetRuleStr());
        ChartState my_state;
        RuleScore<lm::ngram::Model> my_rule_score(*lm_, my_state);
        BOOST_FOREACH(int trg_id, id_edge->GetTrgWords()) {
            if(trg_id < 0) {
                int curr_id = -1 - trg_id;
                // vector<HyperNode*> nodes;
                // Get the chart for the particular node we're interested in
                const ChartEntry & my_entry = BuildChart(parse, chart, states, id_edge->GetTail(curr_id)->GetId(), rule_graph);
                // From the node, get the appropriately ranked node
                int edge_pos = id_str[curr_id+1];
                HyperNode * chart_node = my_entry[edge_pos];
                next_edge->AddTail(chart_node);
                // If we have not gotten to the end, insert a new value into the queue
                if(edge_pos+1 < (int)my_entry.size()) {
                    GenericString<int> next_str = id_str;
                    next_str[curr_id+1]++;
                    if(finished.find(next_str) == finished.end()) {
                        finished.insert(next_str);
                        double next_score = top_score - my_entry[edge_pos]->CalcViterbiScore() + my_entry[edge_pos+1]->CalcViterbiScore();
                        hypo_queue.push(make_pair(next_score, next_str));
                    }
                }
                // Add that edge to our non-terminal
                const ChartState & child_state = states[chart_node->GetId()];
                // cerr << " Adding node context " << *chart_node << " : " << PrintContext(child_state.left) << ", " << PrintContext(child_state.right) << endl;
                my_rule_score.NonTerminal(child_state, 0);
            } else {
                // cerr << " Adding word " << Dict::WSym(trg_id) << endl;
                // Re-index vocabulary
                my_rule_score.Terminal(lm_->GetVocabulary().Index(Dict::WSym(trg_id)));
            }
        }
        double lm_score = my_rule_score.Finish();
        // Retrieve the hypothesis
        // cerr << " LM prob "<<lm_score<<" for: (id_str=" << id_str << ") @ " << PrintContext(my_state.left) << ", " << PrintContext(my_state.right) << endl;
        map<ChartState, HyperNode*>::iterator it = hypo_comb.find(my_state);
        HyperNode * next_node;
        if(it == hypo_comb.end()) {
            // Create a new copy of the current node
            next_node = new HyperNode(nodes[id]->GetSym(), -1, nodes[id]->GetSpan());
            rule_graph.AddNode(next_node);
            states.push_back(my_state);
            hypo_comb.insert(make_pair(my_state, next_node));
            chart[id]->push_back(next_node);
        } else {
            next_node = it->second;
        }
        next_node->SetViterbiScore(max(next_node->GetViterbiScore(),top_score+lm_score*lm_weight_));
        next_edge->SetHead(next_node);
        // Sort the tails in source order
        sort(next_edge->GetTails().begin(), next_edge->GetTails().end(), NodeSrcLess());
        next_edge->SetScore(id_edge->GetScore() + lm_score * lm_weight_);
        if(lm_score != 0.0)
            next_edge->GetFeatures().insert(make_pair(feature_id, lm_score));
        rule_graph.AddEdge(next_edge);
        next_node->AddEdge(next_edge);
        // cerr << " HERE @ " << *next_node << ": " << top_score<<"+"<<lm_score<<"*"<<lm_weight<<" == " << top_score+lm_score*lm_weight << endl;
        // cerr << " Updated node: " << *next_node << endl;
    }
    sort(chart[id]->begin(), chart[id]->end(), NodeScoreMore());
    if(chart_limit_ > 0 && (int)chart[id]->size() > chart_limit_)
        chart[id]->resize(chart_limit_);
    return *chart[id];
}

// Intersect this rule_graph with a language model, using cube pruning to control
// the overall state space.
using namespace lm::ngram;
HyperGraph * LMComposerBU::TransformGraph(const HyperGraph & parse) const {
    // Get the nodes from the parse
    const vector<HyperNode*> & nodes = parse.GetNodes();
    // Create the chart
    //  contains one vector for each node in the old rule_graph
    //   each element of the vector is a node in the new rule_graph
    //   these must be sorted in ascending order of Viterbi probability
    vector<shared_ptr<ChartEntry> > chart(nodes.size());
    // This contains the chart states, indexed by node IDs in the new rule graph
    vector<ChartState> states;
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(parse.GetWords());
    // Add the root node and its corresponding state
    int len = (nodes.size() > 0) ? nodes[0]->GetSpan().second : 0;
    HyperNode * root = new HyperNode(Dict::WID("LMROOT"), -1, make_pair(0,len));
    ret->AddNode(root);
    if(parse.NumNodes() == 0) return ret;
    states.resize(1);
    // Build the chart
    BuildChart(parse, chart, states, 0, *ret);
    // Build the final nodes
    BOOST_FOREACH(HyperNode * node, *chart[0]) {
        HyperEdge * edge = new HyperEdge(root);
        edge->AddTrgWord(-1);
        ChartState my_state;
        RuleScore<lm::ngram::Model> my_rule_score(*lm_, my_state);
        my_rule_score.BeginSentence();
        my_rule_score.NonTerminal(states[node->GetId()], 0);
        my_rule_score.Terminal(lm_->GetVocabulary().Index("</s>"));
        double my_score = my_rule_score.Finish();
        edge->AddTail(node);
        if(my_score != 0.0) {
            edge->AddFeature(Dict::WID(feature_name_), my_score);
            edge->SetScore(my_score * lm_weight_);
        }
        ret->AddEdge(edge);
        root->AddEdge(edge);
        root->SetViterbiScore(max(root->GetViterbiScore(), node->GetViterbiScore() + edge->GetScore()));
    }
    return ret;
}
