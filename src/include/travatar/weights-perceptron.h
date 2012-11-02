#ifndef WEIGHTS_PERCEPTRON_H__
#define WEIGHTS_PERCEPTRON_H__

#include <travatar/weights-pairwise.h>

namespace travatar {

class WeightsPerceptron : public WeightsPairwise {

public:
    WeightsPerceptron() : WeightsPairwise() { }
    WeightsPerceptron(const SparseMap & current) : WeightsPairwise(current) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseMap & oracle, double oracle_score, double oracle_loss,
        const SparseMap & system, double system_score, double system_loss
    ) {
        if(system_score >= oracle_score) {
            current_ += (oracle - system);
        }
    }


protected:

};

}

#endif
