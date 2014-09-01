#include <travatar/trimmer-nbest.h>
#include <travatar/nbest-list.h>
#include <travatar/sentence.h>
#include <travatar/hyper-graph.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace travatar;
using namespace boost;

// Find which nodes and edges should be active in this hypergraph
void TrimmerNbest::FindActive(const HyperGraph & hg,
                std::map<int,int> & active_nodes,
                std::map<int,int> & active_edges) const {
    // Always preserve the root node
    if(hg.GetNodes().size() != 0)
        Trimmer::AddId(active_nodes, 0);
    // Get the n-best
    // TODO: const_cast is a bit dangerous, but let's leave it for now
    NbestList nbest = const_cast<HyperGraph&>(hg).GetNbest(n_);;
    BOOST_FOREACH(boost::shared_ptr<HyperPath> & hyp, nbest) {
        BOOST_FOREACH(HyperEdge * edge, hyp->GetEdges()) {
            Trimmer::AddId(active_nodes, edge->GetHead()->GetId());
            Trimmer::AddId(active_edges, edge->GetId());
            BOOST_FOREACH(HyperNode * tail, edge->GetTails())
                Trimmer::AddId(active_nodes, tail->GetId());
        }
    }
}

