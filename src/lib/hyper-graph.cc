#include <lm/left.hh>
#include <lm/model.hh>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>
#include <travatar/generic-string.h>
#include <boost/shared_ptr.hpp>
#include <boost/tr1/unordered_set.hpp>
#include <queue>
#include <map>

using namespace std;
using namespace travatar;
using namespace boost;
using namespace lm::ngram;


// Refresh the pointers to head and tail nodes so they point to
// nodes in a new HyperGraph. Useful when copying nodes
void HyperEdge::RefreshPointers(HyperGraph & new_graph) {
    head_ = new_graph.GetNode(head_->GetId());
    for(int i = 0; i < (int)tails_.size(); i++)
        tails_[i] = new_graph.GetNode(tails_[i]->GetId());
}


// Refresh the pointers to head and tail nodes so they point to
// nodes in a new HyperGraph. Useful when copying nodes
void HyperNode::RefreshPointers(HyperGraph & new_graph) {
    for(int i = 0; i < (int)edges_.size(); i++)
        edges_[i] = new_graph.GetEdge(edges_[i]->GetId());
}


// Check to make sure that two hyperedges are equal
bool HyperEdge::operator==(const HyperEdge & rhs) const {
    if(id_ != rhs.id_ ||
       (head_==NULL) != (rhs.head_==NULL) ||
       (head_!=NULL && head_->GetId() != rhs.head_->GetId()) ||
       (tails_.size() != rhs.tails_.size()))
        return false;
    for(int i = 0; i < (int)tails_.size(); i++)
       if((tails_[i]==NULL) != (rhs.tails_[i]==NULL) ||
          (tails_[i]!=NULL && tails_[i]->GetId() != rhs.tails_[i]->GetId()))
          return false;
    for(int i = 0; i < (int)fragment_edges_.size(); i++)
       if((fragment_edges_[i]==NULL) != (rhs.fragment_edges_[i]==NULL) ||
          (fragment_edges_[i]!=NULL && fragment_edges_[i]->GetId() != rhs.fragment_edges_[i]->GetId()))
          return false;
    if(features_ != rhs.features_)
        return false;
    return true;
}

// Output for a hyperedge in JSON format
void HyperEdge::Print(std::ostream & out) const {
    out << "{\"id\": "<<id_<<", \"head\": "<<SafeReference(head_).GetId();
    if(tails_.size()) {
        out << ", \"tails\": [";
        for(int i = 0; i < (int)tails_.size(); i++)
            out << tails_[i]->GetId() << ((i == (int)tails_.size()-1) ? "]" : ", ");
    }
    if(features_.size())
        out << ", \"features\": " << features_;
    out << "}";
}

// Check to make sure that two hypernodes are equal
bool HyperNode::operator==(const HyperNode & rhs) const {
    if(id_ != rhs.id_ || src_span_ != rhs.src_span_ ||
       sym_ != rhs.sym_ || edges_.size() != rhs.edges_.size())
        return false;
    for(int i = 0; i < (int)edges_.size(); i++)
       if((edges_[i]==NULL) != (rhs.edges_[i]==NULL) ||
          (edges_[i]!=NULL && edges_[i]->GetId() != rhs.edges_[i]->GetId()))
          return false;
    if(trg_span_ != rhs.trg_span_)
       return false;
    return true;
}

// Output for a hypernode in JSON format
void HyperNode::Print(std::ostream & out) const {
    out << "{\"sym\": ";
    if(sym_==-1)
        out << "null";
    else 
        out << "\""<<Dict::WSym(sym_)<<"\"";
    out << ", \"span\": "<<src_span_<<", \"id\": "<<id_;
    if(edges_.size()) {
        out << ", \"edges\": [";
        for(int i = 0; i < (int)edges_.size(); i++)
            out << edges_[i]->GetId() << ((i == (int)edges_.size()-1) ? "]" : ", ");
    }
    if(has_trg_span_) {
        out << ", \"trg_span\": [";
        int num = 0;
        BOOST_FOREACH(int v, trg_span_)
            out << (num++ != 0?", ":"") << v;
        out << "]";
    }
    if(viterbi_score_ != -DBL_MAX)
        out << ", \"viterbi\": " << viterbi_score_;
    out << "}";
}


// Check to make sure two hypergraphs are equal
int HyperGraph::CheckEqual(const HyperGraph & rhs) const {
    return CheckPtrVector(nodes_, rhs.nodes_) &&
           CheckPtrVector(edges_, rhs.edges_) &&
           CheckVector(words_, rhs.words_);
}

const set<int> & HyperNode::CalculateTrgSpan(
        const vector<set<int> > & word_spans) {
    // Memoized recursion
    if(has_trg_span_) return trg_span_;
    has_trg_span_ = true;
    // If this is terminal, simply set to aligned values
    if(IsTerminal()) {
        // Skip null values
        if(src_span_.first < (int)word_spans.size())
            trg_span_ = word_spans[src_span_.first];
    } else {
        // First, calculate all the spans
        trg_span_ = set<int>();
        BOOST_FOREACH(HyperNode* child, GetEdge(0)->GetTails()) {
            BOOST_FOREACH(int val, child->CalculateTrgSpan(word_spans)) {
                trg_span_.insert(val);
            }
        }
    }
    return trg_span_;
}

// Calculate whether each node is on the frontier or not.
// At the moment, we will treat terminals as non-frontier nodes, and only
// extract words that are rooted at a non-terminal.
HyperNode::FrontierType HyperNode::CalculateFrontier(
                   const vector<set<int> > & src_spans,
                   const set<int> & complement) {
    if(frontier_ != UNSET_FRONTIER) return frontier_;
    if(IsTerminal()) return (frontier_ = HyperNode::NOT_FRONTIER);
    // Check if this is in the frontier
    CalculateTrgSpan(src_spans);
    // We define null-aligned words to not be on the frontier
    if(trg_span_.size() != 0) {
        frontier_ = HyperNode::IS_FRONTIER;
        for(int i = *trg_span_.begin(); frontier_ == HyperNode::IS_FRONTIER && i <= *trg_span_.rbegin(); i++)
            if(complement.find(i) != complement.end())
                frontier_ = HyperNode::NOT_FRONTIER;
    } else {
        frontier_ = HyperNode::NOT_FRONTIER;
    }
    // For all other nodes
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        vector<HyperNode*> & tails = edge->GetTails();
        BOOST_FOREACH(HyperNode* child, tails) {
            if(child->IsFrontier() != HyperNode::UNSET_FRONTIER) continue;
            set<int> my_comp = complement;
            BOOST_FOREACH(HyperNode* child2, tails) {
                if(child != child2) {
                    BOOST_FOREACH(int pos, child2->CalculateTrgSpan(src_spans))
                        my_comp.insert(pos);
                }
            }
            child->CalculateFrontier(src_spans, my_comp);
        }
    }
    return frontier_;
}

class PathScoreLess {
public:
    bool operator()(const shared_ptr<HyperPath> x, const shared_ptr<HyperPath> y) {
        if(abs(x->GetScore() - y->GetScore()) > 1e-6) return x->GetScore() < y->GetScore();
        return x->GetEdges().size() < y->GetEdges().size();
    }
};

vector<shared_ptr<HyperPath> > HyperGraph::GetNbest(int n) {
    priority_queue<shared_ptr<HyperPath>,
                   vector<shared_ptr<HyperPath> >, 
                   PathScoreLess> paths;
    shared_ptr<HyperPath> init_path(new HyperPath);
    init_path->PushNode(nodes_[0]);
    init_path->AddScore(nodes_[0]->CalcViterbiScore());
    paths.push(init_path);
    vector<shared_ptr<HyperPath> > ret;
    while(paths.size() > 0 && (int)ret.size() < n) {
        shared_ptr<HyperPath> curr_path = paths.top();
        paths.pop();
        HyperNode * node = curr_path->PopNode();
        if(node == NULL) {
            ret.push_back(curr_path);
        } else {
            curr_path->AddScore(-1*node->CalcViterbiScore());
            // Expand each different edge
            BOOST_FOREACH(HyperEdge * edge, node->GetEdges()) {
                // Create a new path that is a copy of the old one, and add
                // the edge and its corresponding score
                shared_ptr<HyperPath> next_path(new HyperPath(*curr_path));
                next_path->AddEdge(edge);
                next_path->AddScore(edge->GetScore());
                BOOST_FOREACH(HyperNode * node, edge->GetTails())
                    next_path->AddScore(node->CalcViterbiScore());
                // Add the nodes in reverse order, to ensure that we
                // are doing a depth-first left-to-right traversal
                BOOST_REVERSE_FOREACH(HyperNode * tail, edge->GetTails())
                    next_path->PushNode(tail);
                paths.push(next_path);
            }
        }
        
    }
    return ret;
}


// Check to make sure that two hyperpaths are equal
bool HyperPath::operator==(const HyperPath & rhs) const {
    for(int i = 0; i < (int)edges_.size(); i++)
       if((edges_[i]==NULL) != (rhs.edges_[i]==NULL) ||
          (edges_[i]!=NULL && edges_[i]->GetId() != rhs.edges_[i]->GetId()))
          return false;
    for(int i = 0; i < (int)remaining_nodes_.size(); i++)
       if((remaining_nodes_[i]==NULL) != (rhs.remaining_nodes_[i]==NULL) ||
          (remaining_nodes_[i]!=NULL && remaining_nodes_[i]->GetId() != rhs.remaining_nodes_[i]->GetId()))
          return false;
    return abs(score_ - rhs.score_) < 1e-5;
}

// Output for a hyperedge in JSON format
void HyperPath::Print(std::ostream & out) const {
    out << "{\"edges\": [";
    for(int i = 0; i < (int)edges_.size(); i++)
        out << (i != 0 ? ", " : "") << edges_[i]->GetId();
    out << "], \"score\": " << score_;
    if(remaining_nodes_.size()) {
        out << ", \"remaining_nodes\": [";
        for(int i = 0; i < (int)remaining_nodes_.size(); i++)
            out << remaining_nodes_[i]->GetId() << ((i == (int)remaining_nodes_.size()-1) ? "]" : ", ");
    }
    out << "}";
}


// Calculate the features for this path by simply adding up all the features
SparseMap HyperPath::CalcFeatures() {
    SparseMap ret;
    BOOST_FOREACH(HyperEdge* edge, edges_)
        ret += edge->GetFeatures();
    return ret;
}

// Calculate the translation of the path
vector<WordId> HyperPath::CalcTranslation(int & idx, const std::vector<WordId> & src_words) {
    vector<vector<WordId> > child_trans;
    int my_id = idx++;
    BOOST_FOREACH(HyperNode * tail, SafeAccess(edges_, my_id)->GetTails()) {
        if(tail != edges_[idx]->GetHead())
            THROW_ERROR("Unmatching hyper-nodes " << *tail);
        child_trans.push_back(CalcTranslation(idx, src_words));
    }
    vector<WordId> ret;
    BOOST_FOREACH(int wid, edges_[my_id]->GetTrgWords()) {
        // Special handling of unknowns
        if(wid == Dict::WID("<unk>")) {
            // For terminals, map all source words into the target
            if(edges_[my_id]->GetTails().size() == 0) {
                pair<int,int> span = edges_[my_id]->GetHead()->GetSpan();
                for(int i = span.first; i < span.second; i++)
                    ret.push_back(src_words[i]);
            // For non-terminals, map in order
            } else {
                BOOST_FOREACH(const vector<int> & vec, child_trans)
                    BOOST_FOREACH(int next_wid, vec)
                        ret.push_back(next_wid);
            }
        } else if(wid >= 0) {
            ret.push_back(wid);
        } else {
            BOOST_FOREACH(int next_wid, child_trans[-1 - wid])
                ret.push_back(next_wid);
        }
    }
    return ret;
}

// Score each edge in the graph
void HyperGraph::ScoreEdges(const SparseMap & weights) {
    BOOST_FOREACH(HyperEdge * edge, edges_)
        edge->SetScore(edge->GetFeatures() * weights);
}

class QueueEntry {
public:
    QueueEntry(double score, double lm_score, const GenericString<int> & id) :
        score_(score), lm_score_(lm_score), id_(id) { }
    // Score of the entry
    double score_, lm_score_;
    GenericString<int> id_;
};

class NodeScoreMore {
public:
    bool operator()(const HyperNode* x, const HyperNode* y) {
        return x->GetViterbiScore() > y->GetViterbiScore();
    }
};

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

const ChartEntry & HyperGraph::BuildChart(
                    const Model & lm, 
                    double lm_weight,
                    int stack_pop_limit, 
                    vector<shared_ptr<ChartEntry> > & chart, 
                    vector<ChartState> & states, 
                    int id,
                    HyperGraph & graph) {
    // Don't build already finished charts
    if(chart[id].get() != NULL) return *chart[id];
    chart[id].reset(new ChartEntry);
    // The priority queue of values yet to be expanded
    priority_queue<pair<double, GenericString<int> > > hypo_queue;
    // The hypothesis combination map
    map<ChartState, HyperNode*> hypo_comb;
    // The set indicating already-expanded combinations
    unordered_set<GenericString<int>, GenericHash<GenericString<int> > > finished;
    // For each edge outgoing from this node, add its best hypothesis
    // to the chart
    const vector<HyperEdge*> & node_edges = nodes_[id]->GetEdges();
    for(int i = 0; i < (int)node_edges.size(); i++) {
        HyperEdge * my_edge = node_edges[i];
        GenericString<int> q_id(my_edge->GetTails().size()+1);
        q_id[0] = i;
        double viterbi_score = my_edge->GetScore();
        for(int j = 1; j < (int)q_id.length(); j++) {
            q_id[j] = 0;
            const ChartEntry & my_entry = BuildChart(lm, lm_weight, stack_pop_limit, chart, states, my_edge->GetTail(j-1)->GetId(), graph);
            viterbi_score += my_entry[0]->CalcViterbiScore();
        }
        hypo_queue.push(MakePair(viterbi_score, q_id));
    }
    // For each edge on the queue, process it
    int num_popped = 0;
    while(hypo_queue.size() != 0) {
        if(num_popped++ >= stack_pop_limit) break;
        // Get the score, id string, and edge
        double top_score = hypo_queue.top().first;
        GenericString<int> id_str = hypo_queue.top().second;
        const HyperEdge * id_edge = nodes_[id]->GetEdge(id_str[0]);
        // cerr << "Processing id="<<id<<", id_str="<<id_str<<", id_edge="<<*id_edge<<endl;
        hypo_queue.pop();
        // Find the chart state and LM probability
        HyperEdge * next_edge = new HyperEdge;
        ChartState my_state;
        RuleScore<lm::ngram::Model> my_rule_score(lm, my_state);
        BOOST_FOREACH(int trg_id, id_edge->GetTrgWords()) {
            if(trg_id < 0) {
                int curr_id = -1 - trg_id;
                vector<HyperNode*> nodes;
                // Get the chart for the particular node we're interested in
                const ChartEntry & my_entry = BuildChart(lm, lm_weight, stack_pop_limit, chart, states, id_edge->GetTail(curr_id)->GetId(), graph);
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
                        hypo_queue.push(MakePair(next_score, next_str));
                    }
                }
                // Add that edge to our non-terminal
                const ChartState & child_state = SafeAccess(states, chart_node->GetId());
                // cerr << " Adding node context " << *chart_node << " : " << PrintContext(child_state.left) << ", " << PrintContext(child_state.right) << endl;
                my_rule_score.NonTerminal(child_state, 0);
            } else {
                // cerr << " Adding word " << Dict::WSym(trg_id) << endl;
                // Re-index vocabulary
                my_rule_score.Terminal(lm.GetVocabulary().Index(Dict::WSym(trg_id)));
            }
        }
        double lm_score = my_rule_score.Finish();
        // Retrieve the hypothesis
        // cerr << " LM prob "<<lm_score<<" for: (id_str=" << id_str << ") @ " << PrintContext(my_state.left) << ", " << PrintContext(my_state.right) << endl;
        map<ChartState, HyperNode*>::iterator it = hypo_comb.find(my_state);
        HyperNode * next_node;
        if(it == hypo_comb.end()) {
            // Create a new copy of the current node
            next_node = new HyperNode(nodes_[id]->GetSym(), nodes_[id]->GetSpan());
            graph.AddNode(next_node);
            states.push_back(my_state);
            hypo_comb.insert(MakePair(my_state, next_node));
            chart[id]->push_back(next_node);
        } else {
            next_node = it->second;
        }
        next_node->SetViterbiScore(max(next_node->GetViterbiScore(),top_score+lm_score*lm_weight));
        next_edge->SetHead(next_node);
        if(lm_score != 0.0)
            next_edge->GetFeatures().insert(MakePair(Dict::WID("lm"), lm_score));
        graph.AddEdge(next_edge);
        next_node->AddEdge(next_edge);
        // cerr << " Updated node: " << *next_node << endl;
    }
    sort(chart[id]->begin(), chart[id]->end(), NodeScoreMore());
    return *chart[id];
}

// Intersect this graph with a language model, using cube pruning to control
// the overall state space.
using namespace lm::ngram;
HyperGraph * HyperGraph::IntersectWithLM(const Model & lm, double lm_weight, int stack_pop_limit) {
    // Create the chart
    //  contains one vector for each node in the old graph
    //   each element of the vector is a node in the new graph
    //   these must be sorted in ascending order of Viterbi probability
    vector<shared_ptr<ChartEntry> > chart(nodes_.size());
    // This contains the chart states, indexed by node IDs in the new graph
    vector<ChartState> states;
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(words_);
    // Add the root node and its corresponding state
    HyperNode * root = new HyperNode(Dict::WID("LMROOT"), MakePair(0, nodes_[0]->GetSpan().second));
    ret->AddNode(root);
    states.resize(1);
    // Build the chart
    BuildChart(lm, lm_weight, stack_pop_limit, chart, states, 0, *ret);
    // Build the final nodes
    BOOST_FOREACH(HyperNode * node, *chart[0]) {
        HyperEdge * edge = new HyperEdge(root);
        ChartState my_state;
        RuleScore<lm::ngram::Model> my_rule_score(lm, my_state);
        my_rule_score.BeginSentence();
        my_rule_score.NonTerminal(states[node->GetId()], 0);
        my_rule_score.Terminal(Dict::WID("</s>"));
        double my_score = my_rule_score.Finish();
        edge->SetScore(my_score*lm_weight);
        edge->AddTail(node);
        if(my_score != 0.0)
            edge->AddFeature(Dict::WID("lm"), my_score);
        ret->AddEdge(edge);
        root->AddEdge(edge);
    }
    return ret;
}

void HyperEdge::SetRule(const TranslationRule * rule) {
    rule_str_ = rule->GetSrcStr();
    features_ = rule->GetFeatures();
    trg_words_ = rule->GetTrgWords();
}
