#ifndef WEIGHTS_DELAYED_PERCEPTRON_H__
#define WEIGHTS_DELAYED_PERCEPTRON_H__

#include <travatar/weights-pairwise.h>

namespace travatar {

class WeightsDelayedPerceptron : public WeightsPairwise {

public:
    WeightsDelayedPerceptron() : WeightsPairwise() { }
    WeightsDelayedPerceptron(const SparseMap & current) : WeightsPairwise(current), final_() { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseMap & oracle, double oracle_score, double oracle_loss,
        const SparseMap & system, double system_score, double system_loss
    ) {
        if(system_score >= oracle_score) {
            final_ += (oracle - system);
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
