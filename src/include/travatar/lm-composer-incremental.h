#ifndef LM_COMPOSER_INCREMENTAL_H__
#define LM_COMPOSER_INCREMENTAL_H__

#include <string>
#include <boost/shared_ptr.hpp>
#include <lm/left.hh>
#include <search/applied.hh>
#include <search/edge.hh>
#include <search/edge_generator.hh>
#include <travatar/lm-composer.h>

namespace travatar {

class HyperNode;
class HyperGraph;

// A language model composer that uses incremental search with state grouping
// to perform search:
//  Kenneth Heafield; Philipp Koehn; Alon Lavie
//  Grouping Language Model Boundary Words to Speed K–Best Extraction from Hypergraphs
//  NAACL 2013
class LMComposerIncremental : public LMComposer {

protected:

    // The maximum number of stack items popped during search
    int stack_pop_limit_;

public:
    LMComposerIncremental(lm::ngram::Model * lm) : LMComposer(lm), stack_pop_limit_(0) { }
    virtual ~LMComposerIncremental() { }

    // Intersect this graph with a language model, using incremental search
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    // Accessors
    int GetStackPopLimit() const { return stack_pop_limit_; }
    void SetStackPopLimit(double stack_pop_limit) { stack_pop_limit_ = stack_pop_limit; }

protected:

    // Calculate a single vertex
    search::Vertex* CalculateVertex(
                    const HyperGraph & parse, std::vector<search::Vertex*> & verticies,
                    search::Context<lm::ngram::Model> & context, search::SingleBest & best,
                    int id) const;

};

}

#endif
