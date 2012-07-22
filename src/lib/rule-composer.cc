#include <travatar/rule-composer.h>

using namespace std;
using namespace boost;
using namespace travatar;

// Binarize the graph to the right
HyperGraph * RuleComposer::TransformGraph(const HyperGraph & hg) {
    HyperGraph * ret = new HyperGraph(hg);
    const vector<HyperNode*> & nodes = ret->GetNodes();
    // Add the order-1 edges
    vector<vector<HyperEdge*> > last_edges, min_edges;
    BOOST_FOREACH(HyperNode * node, nodes)
        last_edges.push_back(node->GetEdges());
    min_edges = last_edges;
    // For order two and up, compose each higher order with the order 1
    for(int comp_ord = 2; comp_ord <= order_; comp_ord++) {
        vector<vector<HyperEdge*> > next_edges(nodes.size());
        for(int nid = 0; nid < (int)nodes.size(); nid++) {
            BOOST_FOREACH(HyperEdge* par_edge, last_edges[nid]) {
                const vector<HyperNode*> & par_tails = par_edge->GetTails();
                for(int tid = 0; tid < (int)par_tails.size(); tid++) {
                    BOOST_FOREACH(HyperEdge * child_edge, min_edges[par_tails[tid]->GetId()]) {
                        HyperEdge * comp = RuleComposer::ComposeEdge(*par_edge, *child_edge, tid);
                        next_edges[nid].push_back(comp);
                        ret->AddEdge(comp);
                        ret->GetNode(nid)->AddEdge(comp);
                    }
                }
            }
        }
        last_edges = next_edges;
    }
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
