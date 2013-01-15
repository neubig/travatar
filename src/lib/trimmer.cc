#include <travatar/trimmer.h>
#include <travatar/hyper-graph.h>
#include <travatar/util.h>
#include <boost/foreach.hpp>
#include <map>

using namespace std;
using namespace travatar;
using namespace boost;

HyperGraph * Trimmer::TransformGraph(const HyperGraph & hg) {
    std::map<int,int> active_nodes, active_edges;
    // FindActive(hg, active_nodes, active_edges);
    THROW_ERROR("FindActive not finished");
    HyperGraph * ret = new HyperGraph(hg);
    // Make the new edge/node arrays
    ret->GetEdges().clear();
    ret->GetEdges().resize(active_edges.size());
    ret->GetNodes().clear();
    ret->GetNodes().resize(active_nodes.size());
    // Add the active edges/nodes
    BOOST_FOREACH(const HyperEdge * edge, hg.GetEdges()) {
        std::map<int,int>::const_iterator it = active_edges.find(edge->GetId());
        if(it != active_edges.end())
            ret->GetEdges()[it->second] = new HyperEdge(*edge);
    }
    BOOST_FOREACH(const HyperNode * node, hg.GetNodes()) {
        std::map<int,int>::const_iterator it = active_nodes.find(node->GetId());
        if(it != active_nodes.end())
            ret->GetNodes()[it->second] = new HyperNode(*node);
    }
    // Replace the links
    BOOST_FOREACH(HyperEdge * edge, ret->GetEdges()) {
        vector<HyperNode*> new_tails;
        BOOST_FOREACH(HyperNode* tail, edge->GetTails())
            new_tails.push_back(ret->GetNodes()[active_nodes[tail->GetId()]]);
        edge->SetTails(new_tails);
    }
    BOOST_FOREACH(HyperNode * node, ret->GetNodes()) {
        vector<HyperEdge*> new_edges;
        BOOST_FOREACH(HyperEdge* edge, node->GetEdges())
            new_edges.push_back(ret->GetEdges()[active_edges[edge->GetId()]]);
        node->SetEdges(new_edges);
    }
    return ret;
}
