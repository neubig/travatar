#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>
#include <boost/shared_ptr.hpp>
#include <queue>

using namespace std;
using namespace travatar;
using namespace boost;


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
       (rule_==NULL) != (rhs.rule_==NULL) ||
       (rule_!=NULL && *rule_ != *rhs.rule_) ||
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
    if(rule_ != NULL)
        out << ", \"rule\": " << *rule_;
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

class ComparePathScore {
public:
    bool operator()(const shared_ptr<HyperPath> x, const shared_ptr<HyperPath> y) {
        return x->GetScore() < y->GetScore();
    }
};

vector<shared_ptr<HyperPath> > HyperGraph::GetNbest(int n) {
    priority_queue<shared_ptr<HyperPath>,
                   vector<shared_ptr<HyperPath> >, 
                   ComparePathScore> paths;
    shared_ptr<HyperPath> init_path(new HyperPath);
    init_path->PushNode(nodes_[0]);
    init_path->AddScore(nodes_[0]->GetViterbiScore());
    paths.push(init_path);
    vector<shared_ptr<HyperPath> > ret;
    while(paths.size() > 0 && (int)ret.size() < n) {
        shared_ptr<HyperPath> curr_path = paths.top();
        paths.pop();
        HyperNode * node = curr_path->PopNode();
        if(node == NULL) {
            ret.push_back(curr_path);
        } else {
            curr_path->AddScore(-1*node->GetViterbiContribution());
            // Expand each different edge
            BOOST_FOREACH(HyperEdge * edge, node->GetEdges()) {
                // Create a new path that is a copy of the old one, and add
                // the edge and its corresponding score
                shared_ptr<HyperPath> next_path(new HyperPath(*curr_path));
                next_path->AddEdge(edge);
                next_path->AddScore(edge->GetScore());
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
        ret += edge->GetRule()->GetFeatures();
    return ret;
}

// Calculate the translation of the path
vector<WordId> HyperPath::CalcTranslation(int & idx) {
    vector<vector<WordId> > child_trans;
    int my_id = idx++;
    BOOST_FOREACH(HyperNode * tail, edges_[my_id]->GetTails()) {
        if(tail != edges_[idx]->GetHead())
            THROW_ERROR("Unmatching hyper-nodes " << *tail);
        child_trans.push_back(CalcTranslation(idx));
    }
    vector<WordId> ret;
    BOOST_FOREACH(int wid, SafeReference(edges_[my_id]->GetRule()).GetTrgWords()) {
        if(wid >= 0) {
            ret.push_back(wid);
        } else {
            BOOST_FOREACH(int next_wid, child_trans[-1 - wid])
                ret.push_back(next_wid);
        }
    }
    return ret;
}
