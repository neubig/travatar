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

    // An edge, annotated with the size of the original edges composed
    typedef std::pair<int, HyperEdge*>  SizedEdge;

    RuleComposer(int order, int src_lex_span = 0)
            : order_(order), src_lex_span_(src_lex_span) { }
    virtual ~RuleComposer() { }

    // Binarize the graph to the right
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    // Compose two edges together.
    // child must be an edge rooted at the tail_id'th tail of parent
    static HyperEdge * ComposeEdge(const HyperEdge & parent,
                                   const HyperEdge & child,
                                   int tail_id);

protected:

    // Compose two edges together
    void BuildComposedEdges(int id,
                            const std::vector<std::vector<HyperEdge*> > & min_edges,
                            std::vector<std::vector<SizedEdge> > & composed_edges,
                            HyperGraph * ret) const;

    // The order of composition to perform
    int order_;
    // The maximum size of the span to be used on the source side, which can
    // override the limit on composition of rules
    int src_lex_span_;

};

}

#endif
