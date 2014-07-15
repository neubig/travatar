#ifndef UNARY_FLATTENER_RIGHT_H__
#define UNARY_FLATTENER_RIGHT_H__

#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <boost/unordered_map.hpp>
#include <string>

namespace travatar {

class HyperNode;
class HyperGraph;

class UnaryFlattener : public GraphTransformer {

public:

    UnaryFlattener() { }
    virtual ~UnaryFlattener() { }

    // Binarize the graph to the right
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

protected:

};

}

#endif
