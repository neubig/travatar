#ifndef WEIGHTS_ADAGRAD_H__
#define WEIGHTS_ADAGRAD_H__

#include <travatar/real.h>
#include <travatar/weights-perceptron.h>

namespace travatar {

class WeightsAdagrad : public WeightsPerceptron {

public:
    WeightsAdagrad() : WeightsPerceptron() { }
    WeightsAdagrad(const SparseMap & current) : WeightsPerceptron(current) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseVector & oracle, Real oracle_model, Real oracle_eval,
        const SparseVector & system, Real system_model, Real system_eval
    );

protected:

    // The inverse of the diagonal variance matrix
    SparseMap varinv_;

};

}

#endif
