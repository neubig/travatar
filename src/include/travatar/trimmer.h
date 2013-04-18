#ifndef TRIMMER_H__
#define TRIMMER_H__

#include <travatar/graph-transformer.h>
#include <map>

namespace travatar {

class HyperGraph;

// A class to trim hypergraph's size
class Trimmer : public GraphTransformer {

public:
    virtual ~Trimmer() { }

    // Find which nodes and edges should be active in this hypergraph
    virtual void FindActive(const HyperGraph & hg,
                            std::map<int,int> & active_nodes,
                            std::map<int,int> & active_edges) const = 0;

    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

protected:
    // A utility function to add an ID to a map only if it doesn't exist
    static void AddId(std::map<int,int> & id_map, int id);

};

}

#endif
