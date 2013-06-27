#include <travatar/word-splitter.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

vector<string> WordSplitter::RegexSplit(const std::string & str,
                                        const std::string & pad) const {
    boost::sregex_iterator i(str.begin(), str.end(), profile_);
    boost::sregex_iterator j;
    int pos = 0;
    vector<string> ret;
    for(; i != j; ++i) {
        if(i->position() != pos)
            ret.push_back(str.substr(pos, i->position()-pos));
        ret.push_back(pad+i->str()+pad);
        pos = i->position() + i->size();
    }
    if(pos != (int)str.size())
        ret.push_back(str.substr(pos));
    return ret;
}

// Binarize the graph to the right
HyperGraph * WordSplitter::TransformGraph(const HyperGraph & hg) const {
    // First copy the graph
    HyperGraph * ret = new HyperGraph(hg);
    // Find the modified words and map from word indexes
    Sentence new_wids;
    map<int, vector<int> > ids;
    int i = 0, j = 0;
    BOOST_FOREACH(WordId wid, hg.GetWords()) {
        string old_word = Dict::WSym(wid);
        vector<string> new_words;
        if(ignore_.find(wid) != ignore_.end())
            new_words.push_back(old_word);
        else
            new_words = RegexSplit(old_word);
        BOOST_FOREACH(const std::string & word, new_words) {
            ids[i].push_back(j++);
            new_wids.push_back(Dict::WID(word));
        }
        i++;
    }
    if(i == j) return ret;
    // For each node, modify its indexes. 
    map<pair<int, WordId>, vector<HyperNode*> > new_nodes;
    int orig_node_size = ret->NumNodes();
    for(int nid = 0; nid < orig_node_size; nid++) {
        HyperNode* node = ret->GetNode(nid);
        std::pair<int,int> span = node->GetSpan(),
                           new_span(*ids[span.first].begin(),
                                    *ids[span.second-1].rbegin()+1);
        // If it is a non-terminal
        // that has been split, create new nodes as necessary
        if(node->IsPreTerminal() && span.first == span.second-1 && ids[span.first].size() > 1) {
            pair<int, WordId> node_idx = make_pair(span.first,node->GetSym());
            if(new_nodes.find(node_idx) == new_nodes.end()) {
                vector<HyperNode*> split_nodes;
                for(int i = new_span.first; i < new_span.second; i++) {
                    // Add a new pre-terminal
                    HyperNode* new_pre = new HyperNode(node->GetSym(), node->GetTrgSym(), make_pair(i,i+1));
                    ret->AddNode(new_pre);
                    split_nodes.push_back(new_pre);
                    // Add a new terminal
                    HyperNode* new_term = new HyperNode(new_wids[i], -1, make_pair(i,i+1));
                    ret->AddNode(new_term);
                    // Add a new edge
                    HyperEdge* new_edge = new HyperEdge(new_pre);
                    new_edge->AddTail(new_term);
                    ret->AddEdge(new_edge); new_pre->AddEdge(new_edge);
                }
                new_nodes.insert(make_pair(node_idx, split_nodes));
            }
            vector<HyperEdge*> & edges = node->GetEdges();
            HyperEdge* edge = new HyperEdge(node);
            edge->SetId(edges[0]->GetId());
            ret->GetEdges()[edge->GetId()] = edge;
            BOOST_FOREACH(HyperNode* new_node, new_nodes[node_idx]) {
                edge->AddTail(new_node);
            }
            // delete edges[0]; 
            edges[0] = edge;
        }
        node->SetSpan(new_span);
    }
    ret->SetWords(new_wids);
    return ret;
}
