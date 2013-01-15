#ifndef TRIMMER_H__
#define TRIMMER_H__

#include <travatar/graph-transformer.h>

namespace travatar {

class HyperGraph;

// A class to trim hypergraph's size
class Trimmer : public GraphTransformer {

public:
    virtual ~Trimmer() { }

    // Find which nodes and edges should be active in this hypergraph
    virtual void FindActive(const HyperGraph & hg,
                            std::map<int,int> & active_nodes,
                            std::map<int,int> & active_edges) = 0;

    virtual HyperGraph * TransformGraph(const HyperGraph & hg);

};

}

#endif
