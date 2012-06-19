#include <travatar/hyper-graph.h>

using namespace std;
using namespace travatar;

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
    if(trg_span_) {
        out << ", \"trg_span\": [";
        int num = 0;
        BOOST_FOREACH(int v, *trg_span_)
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

// // Output for a graph fragment in JSON format
// void GraphFragment::Print(std::ostream & out) const {
//     out << "{\"score\": "<<score_;
//     if(edges_.size()) {
//         out << ", \"edges\": [";
//         for(int i = 0; i < (int)edges_.size(); i++)
//             out << edges_[i]->GetId() << ((i == (int)edges_.size()-1) ? "]" : ", ");
//     }
//     if(tails_.size()) {
//         out << ", \"tails\": [";
//         for(int i = 0; i < (int)tails_.size(); i++)
//             out << tails_[i]->GetId() << ((i == (int)tails_.size()-1) ? "]" : ", ");
//     }
//     out << "}";
// }

const set<int> * HyperNode::CalculateTrgSpan(
        const vector<set<int> > & word_spans) {
    // Memoized recursion
    if(trg_span_ != NULL) return trg_span_;
    // If this is terminal, simply set to aligned values
    if(IsTerminal()) {
        trg_span_ = new set<int>(word_spans[src_span_.first]);
    } else {
        // First, calculate all the spans
        trg_span_ = new set<int>;
        BOOST_FOREACH(HyperNode* child, GetEdge(0)->GetTails()) {
            BOOST_FOREACH(int val, *child->CalculateTrgSpan(word_spans)) {
                trg_span_->insert(val);
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
    frontier_ = HyperNode::IS_FRONTIER;
    // Check if this is in the frontier
    const set<int>* span = CalculateTrgSpan(src_spans);
    for(set<int>::const_iterator it = span->begin();
        frontier_ == HyperNode::IS_FRONTIER && it != span->end();
        it++)
        if(complement.find(*it) != complement.end())
            frontier_ = HyperNode::NOT_FRONTIER;
    // For all other nodes
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        vector<HyperNode*> & tails = edge->GetTails();
        BOOST_FOREACH(HyperNode* child, tails) {
            if(child->IsFrontier() != HyperNode::UNSET_FRONTIER) continue;
            set<int> my_comp = complement;
            BOOST_FOREACH(HyperNode* child2, tails) {
                if(child != child2) {
                    BOOST_FOREACH(int pos, *child2->CalculateTrgSpan(src_spans))
                        my_comp.insert(pos);
                }
            }
            child->CalculateFrontier(src_spans, my_comp);
        }
    }
    return frontier_;
}
