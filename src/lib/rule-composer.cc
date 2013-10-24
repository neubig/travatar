#include <boost/foreach.hpp>
#include <travatar/util.h>
#include <travatar/rule-composer.h>
#include <travatar/hyper-graph.h>

using namespace std;
using namespace boost;
using namespace travatar;

// Build composed edges
void RuleComposer::BuildComposedEdges(int id,
                        const vector<vector<HyperEdge*> > & min_edges,
                        vector<vector<RuleComposer::SizedEdge> > & composed_edges,
                        HyperGraph * ret) const {
    if(SafeAccess(composed_edges,id).size() != 0) return;
    // Save the node size
    std::pair<int,int> node_span = ret->GetNode(id)->GetSpan();
    int node_len = node_span.second-node_span.first;
    // For each edge coming from this node
    BOOST_FOREACH(HyperEdge* min_edge, min_edges[id]) {
        int start = composed_edges[id].size();
        composed_edges[id].push_back(make_pair(1, min_edge));
        // Compose all edges from all tails
        for(int tail_id = 0; tail_id < min_edge->NumTails(); tail_id++) {
            int tail_left = min_edge->NumTails() - tail_id;
            HyperNode* tail = min_edge->GetTail(tail_id);
            BuildComposedEdges(tail->GetId(), min_edges, composed_edges, ret);
            int end = composed_edges[id].size();
            for(int i = start; i < end; i++) {
                const SizedEdge above = composed_edges[id][i];
                BOOST_FOREACH(const SizedEdge & below, composed_edges[tail->GetId()]) {
                    // If we have reached a point where composing would exceed the limit, break
                    int sum = above.first+below.first;
                    // Compose and add the edge in the standard fashion
                    if(sum <= order_ || 
                       (node_len <= src_lex_span_ && above.second->NumTails() == 1 && below.second->NumTails() == 0)) {
                        HyperEdge * comp = RuleComposer::ComposeEdge(
                                            *above.second, *below.second,
                                            above.second->NumTails()-tail_left);
                        ret->AddEdge(comp);
                        ret->GetNode(id)->AddEdge(comp);
                        composed_edges[id].push_back(make_pair(sum, comp));
                    } else if(src_lex_span_ == 0) {
                        break;
                    }
                }
            }
        }
        
    }
    // Finally, sort the edges by size
    sort(composed_edges[id].begin(), composed_edges[id].end());
}

// Binarize the graph to the right
HyperGraph * RuleComposer::TransformGraph(const HyperGraph & hg) const {
    HyperGraph * ret = new HyperGraph(hg);
    if(order_ == 1 && src_lex_span_ == 0) return ret;
    // Create the edges
    vector<vector<HyperEdge*> > min_edges;
    vector<vector<SizedEdge> > composed_edges(ret->NumNodes());
    // Create sets to remove duplicates
    BOOST_FOREACH(HyperNode * node, ret->GetNodes())
        min_edges.push_back(node->GetEdges());
    if(min_edges.size() > 0)
        BuildComposedEdges(0, min_edges, composed_edges, ret);
    return ret;
}

// Compose two edges together.
// child must be an edge rooted at the tail_id'th tail of parent
HyperEdge * RuleComposer::ComposeEdge(const HyperEdge & parent,
                                      const HyperEdge & child,
                                      int tail_id) {
    // Sanity check
    if(parent.GetTail(tail_id) != child.GetHead())
        THROW_ERROR("ComposeEdge parent tail != child head: " << *parent.GetTail(tail_id) << " != " << *child.GetHead());
    HyperEdge * composed = new HyperEdge;
    // do not set id_
    // cover head_
    composed->SetHead(parent.GetHead());
    // for tails, we need to merge them together in the proper order
    vector<HyperNode*> tails = parent.GetTails();
    tails.erase(tails.begin() + tail_id);
    BOOST_REVERSE_FOREACH(HyperNode* tail, child.GetTails())
        tails.insert(tails.begin() + tail_id, tail);
    composed->SetTails(tails);
    // score should be the sum of the scores
    composed->SetScore(parent.GetScore() + child.GetScore());
    // do not set rule_string_, this will generally be generated after composition
    // set trg_words, replacing the symbol with all the tails
    vector<int> trg_words;
    int child_tails = child.GetTails().size();
    int trg_placeholder = -1 - tail_id;
    BOOST_FOREACH(int trg, parent.GetTrgWords()) {
        if(trg >= 0 || trg > trg_placeholder) {
            trg_words.push_back(trg);
        } else if (trg == trg_placeholder) {
            BOOST_FOREACH(int ctrg, child.GetTrgWords()) {
                if(ctrg >= 0)
                    trg_words.push_back(ctrg);
                else
                    trg_words.push_back(ctrg-tail_id);
            }
        } else {
            trg_words.push_back(trg - child_tails + 1);
        }
    }
    composed->SetTrgWords(trg_words);
    // insert the edges at the appropriate place (the first time any edge starts
    // after them)
    vector<HyperEdge*> fragments;
    const vector<HyperEdge*> & cfrags = child.GetFragmentEdges();
    const vector<HyperEdge*> & pfrags = parent.GetFragmentEdges();
    // find the insertion position
    int pos = 0;
    for(;pos < (int)pfrags.size(); pos++) {
        if(pfrags[pos]->GetHead()->GetSpan().first > cfrags[0]->GetHead()->GetSpan().first)
            break;
        fragments.push_back(pfrags[pos]);
    }
    BOOST_FOREACH(HyperEdge * cfrag, cfrags)
        fragments.push_back(cfrag);
    for(;pos < (int)pfrags.size(); pos++)
        fragments.push_back(pfrags[pos]);
    composed->SetFragmentEdges(fragments);
    
    return composed;
}
