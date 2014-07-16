#include <travatar/lm-composer-bu.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <lm/left.hh>
#include <vector>
#include <queue>
#include <map>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace lm::ngram;

namespace travatar {

inline string PrintContext(const State & context) {
    ostringstream out;
    out << "[";
    for(unsigned i = 0; i < context.length; i++) {
        if(i != 0) out << ", ";
        out << context.words[i];
    }
    out << "]";
    return out.str();
}
inline string PrintContext(const Left & context) {
    ostringstream out;
    out << "[";
    for(unsigned i = 0; i < context.length; i++) {
        if(i != 0) out << ", ";
        out << context.pointers[i];
    }
    out << "]";
    return out.str();
}

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

const ChartEntry & LMComposerBU::BuildChartCubePruning(
                    const HyperGraph & parse,
                    vector<shared_ptr<ChartEntry> > & chart,
                    vector<vector<ChartState> > & states,
                    int id,
                    HyperGraph & rule_graph) const {
    // Save the nodes for easy access
    const vector<HyperNode*> & nodes = parse.GetNodes();
    // Don't build already finished charts
    if(chart[id].get() != NULL) return *chart[id];
    chart[id].reset(new ChartEntry);
    ChartEntry & my_chart = *chart[id].get();
    // The priority queue of values yet to be expanded
    priority_queue<pair<double, GenericString<int> > > hypo_queue;
    // The hypothesis combination map
    map<vector<ChartState>, HyperNode*> hypo_comb;
    map<HyperNode*, vector<ChartState> > hypo_rev;
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
            const ChartEntry & my_entry = BuildChartCubePruning(parse, chart, states, my_edge->GetTail(j-1)->GetId(), rule_graph);
            // For empty nodes, break
            if(my_entry.size() == 0) {
                viterbi_score = -DBL_MAX;
                break;
            } else {
                viterbi_score += my_entry[0]->CalcViterbiScore();
            }
        }
        if(viterbi_score != -DBL_MAX)
            hypo_queue.push(make_pair(viterbi_score, q_id));
    }
    // For each edge on the queue, process it
    int num_popped = 0;
    while(hypo_queue.size() != 0) {
        if(num_popped++ >= stack_pop_limit_) break;
        // Get the score, id string, and edge
        double top_score = hypo_queue.top().first;
        GenericString<int> id_str = hypo_queue.top().second;
        const HyperEdge * id_edge = nodes[id]->GetEdge(id_str[0]);
        // cerr << "Processing ID string: " << id_str << endl;
        hypo_queue.pop();
        // Find the chart state and LM probability
        HyperEdge * next_edge = new HyperEdge;
        next_edge->SetFeatures(id_edge->GetFeatures());
        next_edge->SetTrgData(id_edge->GetTrgData());
        next_edge->SetSrcStr(id_edge->GetSrcStr());
        vector<ChartState> my_state(lm_data_.size());
        // *** Get the data, etc. necessary for scoring
        for(int curr_id = 0; curr_id < (int)id_edge->GetTails().size(); curr_id++) {
            const HyperNode * tail = id_edge->GetTail(curr_id);
            // Get the chart for the particular node we're interested in
            const ChartEntry & my_entry = BuildChartCubePruning(parse, chart, states, tail->GetId(), rule_graph);
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
        }
        // *** Actually step through in target order, scoring
        double total_score = 0;
        vector<SparsePair> lm_features;
        for(int lm_id = 0; lm_id < (int)lm_data_.size(); lm_id++) {
            LMData* data = lm_data_[lm_id];
            RuleScore<lm::ngram::Model> my_rule_score(*data->GetLM(), my_state[lm_id]);
            int unk = 0;
            BOOST_FOREACH(int trg_id, id_edge->GetTrgData()[data->GetFactor()].words) {
                if(trg_id < 0) {
                    int curr_id = -1 - trg_id;
                    // Add that edge to our non-terminal
                    const vector<ChartState> & child_state = states[next_edge->GetTail(curr_id)->GetId()];
                    // cerr << " Adding node context " << *next_edge->GetTail(curr_id) << " : " << PrintContext(child_state[lm_id].left) << ", " << PrintContext(child_state[lm_id].right) << endl;
                    my_rule_score.NonTerminal(child_state[lm_id], 0);
                } else {
                    // cerr << " Adding word " << Dict::WSym(trg_id) << endl;
                    // Re-index vocabulary
                    lm::WordIndex index = data->GetMapping(trg_id);
                    if(index == 0) unk++;
                    my_rule_score.Terminal(index);
                }
            }
            double lm_score = my_rule_score.Finish();
            // Add to the features and the score
            // cerr << " LM prob "<<lm_score<<" for: (id_str=" << id_str << ") @ " << PrintContext(my_state[lm_id].left) << ", " << PrintContext(my_state[lm_id].right) << endl;
            total_score += lm_score * data->GetWeight() + unk * data->GetUnkWeight();
            if(lm_score != 0.0)
                lm_features.push_back(make_pair(data->GetFeatureName(), lm_score));
                // next_edge->GetFeatures()[data->GetFeatureName()] += lm_score;
            if(unk != 0)
                lm_features.push_back(make_pair(data->GetUnkFeatureName(), unk));
                // next_edge->GetFeatures()[data->GetUnkFeatureName()] += unk;
        }
        // Clean up the features
        next_edge->GetFeatures() += SparseVector(lm_features);
        // Retrieve the hypothesis
        map<vector<ChartState>, HyperNode*>::iterator it = hypo_comb.find(my_state);
        HyperNode * next_node;
        if(it == hypo_comb.end()) {
            // Create a new copy of the current node
            next_node = new HyperNode(nodes[id]->GetSym(), -1, nodes[id]->GetSpan());
            // rule_graph.AddNode(next_node);
            // states.push_back(my_state);
            hypo_comb.insert(make_pair(my_state, next_node));
            hypo_rev.insert(make_pair(next_node, my_state));
            my_chart.push_back(next_node);
        } else {
            next_node = it->second;
        }
        next_node->SetViterbiScore(max(next_node->GetViterbiScore(),total_score + top_score));
        next_edge->SetHead(next_node);
        next_edge->SetScore(id_edge->GetScore() + total_score);
        // rule_graph.AddEdge(next_edge);
        next_node->AddEdge(next_edge);
        // cerr << " Updated node: " << *next_node << ", edge score = " << id_edge->GetScore() + total_score << endl;
    }
    sort(my_chart.begin(), my_chart.end(), NodeScoreMore());
    // Destroy all edges/nodes over the chart limit
    if(chart_limit_ > 0 && (int)my_chart.size() > chart_limit_) {
        for(int i = chart_limit_; i < (int)my_chart.size(); i++) {
            BOOST_FOREACH(HyperEdge * edge, my_chart[i]->GetEdges())
                delete edge;
            delete my_chart[i];
        }
        my_chart.resize(chart_limit_);
    }
    // Add the rest of the nodes to the chart
    BOOST_FOREACH(HyperNode * node, my_chart) {
        rule_graph.AddNode(node);
        states.push_back(hypo_rev[node]);
        BOOST_FOREACH(HyperEdge * edge, node->GetEdges())
            rule_graph.AddEdge(edge);
    }
    return my_chart;
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
    vector<vector<ChartState> > states;
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(parse.GetWords());
    // Add the root node and its corresponding state
    int len = (nodes.size() > 0) ? nodes[0]->GetSpan().second : 0;
    HyperNode * root = new HyperNode(Dict::WID("LMROOT"), -1, make_pair(0,len));
    ret->AddNode(root);
    if(parse.NumNodes() == 0) return ret;
    states.resize(1);
    // Build the chart
    BuildChartCubePruning(parse, chart, states, 0, *ret);
    // Build the final nodes
    BOOST_FOREACH(HyperNode * node, *chart[0]) {
        HyperEdge * edge = new HyperEdge(root);
        edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors, CfgData(Sentence(1, -1))));
        double total_score = 0;
        for(int lm_id = 0; lm_id < (int)lm_data_.size(); lm_id++) {
            LMData* data = lm_data_[lm_id];
            ChartState my_state;
            RuleScore<lm::ngram::Model> my_rule_score(*data->GetLM(), my_state);
            my_rule_score.BeginSentence();
            my_rule_score.NonTerminal(states[node->GetId()][lm_id], 0);
            my_rule_score.Terminal(data->GetLM()->GetVocabulary().Index("</s>"));
            double my_score = my_rule_score.Finish();
            edge->AddTail(node);
            if(my_score != 0.0)
                edge->GetFeatures().Add(data->GetFeatureName(), my_score);
            total_score += my_score * data->GetWeight();
        }
        edge->SetScore(total_score);
        ret->AddEdge(edge);
        root->AddEdge(edge);
        root->SetViterbiScore(max(root->GetViterbiScore(), node->GetViterbiScore() + edge->GetScore()));
    }
    return ret;
}
