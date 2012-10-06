#ifndef TUNER_H__
#define TUNER_H__

#include <travatar/hyper-graph.h>

namespace travatar {

// A virtual parent class for tuning strategies
class Tuner {

public:

    // Adjust the weights according to the appropriate strategy using the
    // passed in rule_graph, nbest, words, and reference
    virtual void AdjustWeights(const HyperGraph & graph,
                               const NbestList & nbest,
                               const std::vector<Sentence> & refs,
                               SparseMap & weights) = 0;

    // Get the weights after tuning. By default return the current weights,
    // unless there is a reason not to (i.e. the averaged perceptron)
    virtual SparseMap & GetWeights(SparseMap & def) { return def; }

};

}

#endif
