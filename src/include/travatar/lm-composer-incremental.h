#ifndef LM_COMPOSER_INCREMENTAL_H__
#define LM_COMPOSER_INCREMENTAL_H__

#include <travatar/lm-composer.h>
#include <travatar/hyper-graph.h>
#include <travatar/sentence.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace search {

class PartialEdge;
class NBestComplete;
class Vertex;
template <class V>
class Context;

namespace ngram {
class Model;
}

class Forest {
public:

    // Constructor
    Forest(travatar::WordId lm_id, double lm_weight,
           travatar::WordId lm_unk_id, double lm_unk_weight,
           travatar::WordId root_sym, int factor) : hg(new travatar::HyperGraph),
                lm_id_(lm_id), lm_weight_(lm_weight), lm_unk_id_(lm_unk_id),
                lm_unk_weight_(lm_unk_weight), root_sym_(root_sym), factor_(factor) {
        hg->GetNodes().resize(1, NULL); // Reserve a spot for the root
    }
    // // Constructor
    // Forest(double lm_weight, double lm_unk_weight, int factor) : hg(new travatar::HyperGraph),
    //             lm_weight_(lm_weight), lm_unk_weight_(lm_unk_weight), factor_(factor) {
    //     hg->GetNodes().resize(1, NULL); // Reserve a spot for the root
    // }

    // Destructor. Destroy the hypergraph if it still exists.
    ~Forest() {
        if(hg) delete hg;
    }

    // The edges that we will want to add together
    typedef std::vector<PartialEdge> Combine;

    // At the beginning, just add new edges
    void Add(std::vector<PartialEdge> &existing, PartialEdge add) const;
    
    // Return the hypergraph and set the pointer to null
    travatar::HyperGraph* StealPointer() {
        travatar::HyperGraph* ret = hg;
        hg = NULL;
        return ret;
    }

    // Set the number of LM unknown words for an edge
    void SetLMUnk(int id, int unk) {
        if((int)lm_unks_.size() <= id)
            lm_unks_.resize(id+1, 0);
        lm_unks_[id] = unk;
    }

    // Convert all of the edges together into a node
    NBestComplete Complete(std::vector<PartialEdge> &partial);

  private:
    travatar::HyperGraph* hg;
    travatar::WordId lm_id_;
    double lm_weight_;
    travatar::WordId lm_unk_id_;
    double lm_unk_weight_;
    std::vector<int> lm_unks_;
    travatar::WordId root_sym_;
    int factor_;
    util::Pool pool_;
};

} // namespace search

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

    // The maximum number of edges to add during search
    int edge_limit_;

public:
    LMComposerIncremental(const std::vector<std::string> & str);
    LMComposerIncremental(void * lm, lm::ngram::ModelType type, VocabMap * vocab_map, int factor = 0) :
        LMComposer(lm, type, vocab_map), stack_pop_limit_(0), edge_limit_(1000) { }
    virtual ~LMComposerIncremental() { }

    // Intersect this graph with a language model, using incremental search
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    // Accessors
    int GetStackPopLimit() const { return stack_pop_limit_; }
    void SetStackPopLimit(double stack_pop_limit) { stack_pop_limit_ = stack_pop_limit; }

protected:

    // A templated version of transformgraph
    template <class LMType>
    HyperGraph * TransformGraphTemplate(const HyperGraph & hg) const;

    // Calculate a single vertex
    template <class LMType>
    search::Vertex* CalculateVertex(
                    const HyperGraph & parse, std::vector<search::Vertex*> & verticies,
                    search::Context<LMType> & context, search::Forest & best,
                    int id) const;
    // Calculate the root vertex
    template <class LMType>
    search::Vertex* CalculateRootVertex(
                    std::vector<search::Vertex*> & vertices,
                    search::Context<LMType> & context,
                    search::Forest & best) const;

};

}

#endif
