#ifndef WEIGHTS_DELAYED_PERCEPTRON_H__
#define WEIGHTS_DELAYED_PERCEPTRON_H__

#include <travatar/real.h>
#include <travatar/weights-pairwise.h>

namespace travatar {

class WeightsDelayedPerceptron : public WeightsPairwise {

public:
    WeightsDelayedPerceptron() : WeightsPairwise() { }
    WeightsDelayedPerceptron(const SparseMap & current) : WeightsPairwise(current), final_() { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseVector & oracle, Real oracle_score, Real oracle_loss,
        const SparseVector & system, Real system_score, Real system_loss
    ) {
        if(system_score >= oracle_score) {
            final_ = final_ + (oracle - system);
        }
    }

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        return final_;
    }

protected:
    SparseMap final_; 

};

}

#endif
