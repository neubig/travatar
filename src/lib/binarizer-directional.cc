#include <travatar/binarizer-directional.h>
#include <travatar/generic-string.h>
#include <boost/unordered_map.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

typedef BinarizerDirectional::SNMap SNMap;
HyperNode * BinarizerDirectional::FindIndexedNode(const HyperGraph & hg, HyperGraph & ret, SNMap & snmap, const GenericString<int> & str, WordId xbar) {
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
    snmap.insert(MakePair(str, new_node));
    return new_node;
    
}

// Binarize the graph to the right
HyperGraph * BinarizerDirectional::TransformGraph(const HyperGraph & hg) {
    // First copy the graph
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(hg.GetWords());
    // A map to keep track of nodes that have already been built for this graph
    SNMap built_nodes;
    // Process each edge in the original graph
    BOOST_FOREACH(const HyperEdge * edge, hg.GetEdges()) {
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
            if(tail_str.length() > 0) {
                GenericString<int> str = (dir_ == BINARIZE_RIGHT ? 
                                          tail_str.substr(0,1) :
                                          tail_str.substr(tail_str.length()-1));
                small = FindIndexedNode(hg, *ret, built_nodes, str, xbar);
            }
            if(tail_str.length() > 1) {
                tail_str = (dir_ == BINARIZE_RIGHT ?
                            tail_str.substr(1) :
                            tail_str.substr(0, tail_str.length()-1));
                big = FindIndexedNode(hg, *ret, built_nodes, tail_str, xbar);
            }
            // Create the left and right edges
            HyperEdge * next_edge = new HyperEdge(head);
            if(dir_ == BINARIZE_RIGHT) {
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

    return ret;
}
