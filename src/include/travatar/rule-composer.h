#ifndef RULE_COMPOSER_H__
#define RULE_COMPOSER_H__

#include <travatar/graph-transformer.h>
#include <boost/unordered_map.hpp>
#include <string>

namespace travatar {

class HyperEdge;
class HyperGraph;

// A class for composing rules together
// reference: 
//  Galley "Scalable inference and training of context-rich syntactic
//  translation models"
class RuleComposer : public GraphTransformer {

public:

    RuleComposer(int order) : order_(order) { }
    virtual ~RuleComposer() { }

    // Binarize the graph to the right
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    // Compose two edges together.
    // child must be an edge rooted at the tail_id'th tail of parent
    static HyperEdge * ComposeEdge(const HyperEdge & parent,
                                   const HyperEdge & child,
                                   int tail_id);

protected:

    // The order of composition to perform
    int order_;

};

}

#endif
