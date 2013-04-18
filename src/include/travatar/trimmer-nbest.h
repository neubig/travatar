#ifndef TRIMMER_NBEST_H__
#define TRIMMER_NBEST_H__

#include <travatar/trimmer.h>
#include <map>

namespace travatar {

class HyperGraph;

// A class to trim hypergraph to only include edges that appear in the n-best
class TrimmerNbest : public Trimmer {

public:

    TrimmerNbest(int n) : Trimmer(), n_(n) { }

    // Find which nodes and edges should be active in this hypergraph
    virtual void FindActive(const HyperGraph & hg,
                            std::map<int,int> & active_nodes,
                            std::map<int,int> & active_edges) const;

protected:

    int n_;

};

}

#endif
