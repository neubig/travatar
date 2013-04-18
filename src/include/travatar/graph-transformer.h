#ifndef GRAPH_TRANSFORMER_H__
#define GRAPH_TRANSFORMER_H__

namespace travatar {

class HyperGraph;

class GraphTransformer {

public:
    virtual ~GraphTransformer() { }

    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const = 0;

};

}

#endif
