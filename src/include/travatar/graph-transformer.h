#ifndef GRAPH_TRANSFORMER_H__
#define GRAPH_TRANSFORMER_H__

#include <travatar/hyper-graph.h>

namespace travatar {

class GraphTransformer {

public:
    virtual ~GraphTransformer() { }

    virtual HyperGraph * TransformGraph(const HyperGraph & hg) = 0;

};

}

#endif
