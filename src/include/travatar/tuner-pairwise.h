#ifndef TUNER_PAIRWISE_H__
#define TUNER_PAIRWISE_H__

#include <travatar/hyper-graph.h>

namespace travatar {

// A virtual parent class for tuning strategies
class TunerPairwise : public Tuner {

public:

    // Adjust the weights according to the appropriate strategy using the
    // passed in rule_graph, nbest, words, and reference
    virtual void AdjustWeights(const HyperGraph & graph,
                               const NbestList & nbest,
                               const std::vector<Sentence> & refs,
                               SparseMap & weights) {
        THROW_ERROR("Not implemented yet");
    }

    // Do a pairwise comparison between SparseMaps and update if necessary
    virtual void PairwiseUpdate(
        const SparseMap & oracle_feat_, double oracle_score_,
        const SparseMap & system_feat_, double system_score_,
        SparseMap & weights_) = 0;

};

}

#endif
