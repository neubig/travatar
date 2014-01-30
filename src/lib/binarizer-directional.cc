#include <travatar/binarizer-directional.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

typedef BinarizerDirectional::SNMap SNMap;
HyperNode * BinarizerDirectional::FindIndexedNode(const HyperGraph & hg, HyperGraph & ret, SNMap & snmap, const GenericString<int> & str, WordId xbar) const {
    SNMap::const_iterator it = snmap.find(str);
    if(it != snmap.end()) return it->second;
    HyperNode * new_node = new HyperNode;
    for(int i = 0; i < (int)str.length(); i++) {
        const HyperNode* old_node = hg.GetNode(str[i]);
        new_node->AddSpan(old_node->GetSpan());
        new_node->SetSym(old_node->GetSym());
    }
    if(str.length() > 1) new_node->SetSym(xbar);
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
        GenericString<int> head_str(1);
        head_str[0] = edge->GetHead()->GetId();
        HyperNode * head = FindIndexedNode(hg, *ret, built_nodes, head_str, xbar);
        // Create a generic string holding the tails
        const vector<HyperNode*> & tails = edge->GetTails();
        GenericString<int> tail_str(tails.size());
        for(int i = 0; i < (int)tails.size(); i++)
            tail_str[i] = tails[i]->GetId();
        // Binarize until we have reached an intermediate node that is finished
        bool first = true;
        while(first || (head != NULL && head->GetEdges().size() == 0 && tail_str.length() > 1)) {
            HyperNode *small = NULL, *big = NULL;

            // Choose the direction to do
            Direction dir = dir_;
            if(raise_punc_) {
                // If we are binarizing right, but the last symbol is punctuation
                if(dir_ == BINARIZE_RIGHT &&
                   punc_labs_.find(hg.GetNode(tail_str[tail_str.length()-1])->GetSym()) != punc_labs_.end()) {
                    dir = BINARIZE_LEFT;
                }
                // If we are binarizing left, but the first symbol is punctuation
                if(dir_ == BINARIZE_LEFT &&
                   punc_labs_.find(hg.GetNode(tail_str[0])->GetSym()) != punc_labs_.end())
                    dir = BINARIZE_RIGHT;
            }

            // Find the small and big tails
            if(tail_str.length() > 0) {
                GenericString<int> str = (dir == BINARIZE_RIGHT ? 
                                          tail_str.substr(0,1) :
                                          tail_str.substr(tail_str.length()-1));
                small = FindIndexedNode(hg, *ret, built_nodes, str, xbar);
                // cerr << "sml: " << Dict::WSym(xbar) << endl;
            }
            if(tail_str.length() > 1) {
                tail_str = (dir == BINARIZE_RIGHT ?
                            tail_str.substr(1) :
                            tail_str.substr(0, tail_str.length()-1));
                big = FindIndexedNode(hg, *ret, built_nodes, tail_str, xbar);
                // cerr << "big: " << Dict::WSym(xbar) << endl;
            }
            // Create the left and right edges
            HyperEdge * next_edge = new HyperEdge(head);
            if(dir == BINARIZE_RIGHT) {
                if(small) next_edge->AddTail(small);
                if(big) next_edge->AddTail(big);
                cerr << Dict::WSym(head->GetSym()) << "[" << head->GetSpan().first << "," << head->GetSpan().second-1 << "]("<<head->GetId()<<") => ";
                if(small) cerr << Dict::WSym(small->GetSym()) << "[" << small->GetSpan().first << "," << small->GetSpan().second-1 << "]("<<small->GetId()<<") ";
                if(big)   cerr << Dict::WSym(big->GetSym()) << "[" << big->GetSpan().first << "," << big->GetSpan().second-1 << "]("<<big->GetId()<<")";
                cerr << endl;
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
