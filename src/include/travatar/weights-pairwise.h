#ifndef WEIGHTS_PAIRWISE_H__
#define WEIGHTS_PAIRWISE_H__

#include <travatar/weights.h>

namespace travatar {

class WeightsPairwise : public Weights {

public:

    WeightsPairwise() : Weights() { }
    WeightsPairwise(const SparseMap & current) : Weights(current) { }

    // Adjust the weights according to the n-best list
    virtual void Adjust(EvalMeasure & eval,
                        const std::vector<Sentence> & refs,
                        const NbestList & nbest);

protected:

};

}

#endif
