#ifndef WEIGHTS_PAIRWISE_H__
#define WEIGHTS_PAIRWISE_H__

#include <travatar/weights.h>

namespace travatar {

class WeightsPairwise : public Weights {

public:

    WeightsPairwise() : Weights() { }
    WeightsPairwise(const SparseMap & current) : Weights(current) { }

    // The pairwise weight update rule
    virtual void Update (
        const SparseMap & oracle, double oracle_score, double oracle_loss,
        const SparseMap & system, double system_score, double system_loss
    ) = 0;

    // Adjust the weights according to the n-best list
    virtual void Adjust(const EvalMeasure & eval,
                        const std::vector<Sentence> & refs,
                        const NbestList & nbest);

protected:

};

}

#endif
