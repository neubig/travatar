#include <travatar/binarizer-directional.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

typedef BinarizerDirectional::SNMap SNMap;
HyperNode * BinarizerDirectional::FindIndexedNode(const HyperGraph & hg, HyperGraph & ret, SNMap & snmap, const vector<int> & str, WordId xbar) const {
    SNMap::const_iterator it = snmap.find(str);
    if(it != snmap.end()) return it->second;
    HyperNode * new_node = new HyperNode;
    for(int i = 1; i < (int)str.size(); i++) {
        const HyperNode* old_node = hg.GetNode(str[i]);
        new_node->AddSpan(old_node->GetSpan());
        new_node->SetSym(old_node->GetSym());
    }
    if(str.size() > 2) new_node->SetSym(xbar);
    ret.AddNode(new_node);
    snmap.insert(make_pair(str, new_node));
    return new_node;
    
}

// Binarize the graph to the right
HyperGraph * BinarizerDirectional::TransformGraph(const HyperGraph & hg) const {
    // First copy the graph
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(hg.GetWords());
    // A map to keep track of nodes that have already been built for this graph
    SNMap built_nodes;
    // Process each edge in the original graph
    BOOST_FOREACH(const HyperNode * ehead_node, hg.GetNodes()) {
    BOOST_FOREACH(const HyperEdge * edge, ehead_node->GetEdges()) {
        // Get the symbol for x-bar
        WordId xbar = Dict::WID(Dict::WSym(edge->GetHead()->GetSym())+"'");
        // Get the head node in the new graph
        vector<int> head_str(2);
        head_str[0] = ehead_node->GetSym();
        head_str[1] = edge->GetHead()->GetId();
        HyperNode * head = FindIndexedNode(hg, *ret, built_nodes, head_str, ehead_node->GetSym());
        // Create a generic string holding the tails
        const vector<HyperNode*> & tails = edge->GetTails();
        vector<int> tail_str(tails.size()+1);
        tail_str[0] = xbar;
        for(int i = 0; i < (int)tails.size(); i++)
            tail_str[i+1] = tails[i]->GetId();
        // Binarize until we have reached an intermediate node that is finished
        bool first = true;
        while(first || (head != NULL && head->GetEdges().size() == 0 && tail_str.size() > 2)) {
            HyperNode *small = NULL, *big = NULL;

            // Choose the direction to do
            Direction dir = dir_;
            if(raise_punc_) {
                // If we are binarizing right, but the last symbol is punctuation
                if(dir_ == BINARIZE_RIGHT &&
                   punc_labs_.find(hg.GetNode(tail_str[tail_str.size()-1])->GetSym()) != punc_labs_.end()) {
                    dir = BINARIZE_LEFT;
                }
                // If we are binarizing left, but the first symbol is punctuation
                if(dir_ == BINARIZE_LEFT &&
                   punc_labs_.find(hg.GetNode(tail_str[1])->GetSym()) != punc_labs_.end())
                    dir = BINARIZE_RIGHT;
            }

            // Find the small and big tails
            if(tail_str.size() > 1) {
                vector<int> str = (dir == BINARIZE_RIGHT ? 
                                          VectorSubstr(tail_str, 0,2) :
                                          VectorAdd(VectorSubstr(tail_str, 0,1), VectorSubstr(tail_str, tail_str.size()-1)));
                // Reset the head symbol
                if(str.size() == 2)
                    str[0] = hg.GetNode(str[1])->GetSym();
                small = FindIndexedNode(hg, *ret, built_nodes, str, xbar);
                // cerr << "sml: " << Dict::WSym(xbar) << endl;
            }
            if(tail_str.size() > 2) {
                tail_str = (dir == BINARIZE_RIGHT ?
                            VectorAdd(VectorSubstr(tail_str, 0,1), VectorSubstr(tail_str, 2)) :
                            VectorSubstr(tail_str, 0, tail_str.size()-1));
                // Reset the head symbol
                if(tail_str.size() == 2)
                    tail_str[0] = hg.GetNode(tail_str[1])->GetSym();
                big = FindIndexedNode(hg, *ret, built_nodes, tail_str, xbar);
                // cerr << "big: " << Dict::WSym(xbar) << endl;
            }
            // Create the left and right edges
            HyperEdge * next_edge = new HyperEdge(head);
            if(dir == BINARIZE_RIGHT) {
                if(small) next_edge->AddTail(small);
                if(big) next_edge->AddTail(big);
            } else {
                if(big) next_edge->AddTail(big);
                if(small) next_edge->AddTail(small);
            }
            // Score only the top edge
            if(first) {
                next_edge->SetFeatures(edge->GetFeatures());
                next_edge->SetScore(edge->GetScore());
            }
            ret->AddEdge(next_edge); head->AddEdge(next_edge);
            head = big;
            first = false;
        }        
    }
    }

    return ret;
}
