#ifndef WEIGHTS_ADAGRAD_H__
#define WEIGHTS_ADAGRAD_H__

#include <boost/foreach.hpp>
#include <travatar/weights-perceptron.h>
#include <cmath>

namespace travatar {

class WeightsAdagrad : public WeightsPerceptron {

public:
    WeightsAdagrad() : WeightsPerceptron() { }
    WeightsAdagrad(const SparseMap & current) : WeightsPerceptron(current) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseMap & oracle, double oracle_model, double oracle_eval,
        const SparseMap & system, double system_model, double system_eval
    );

protected:

    // The inverse of the diagonal variance matrix
    SparseMap varinv_;

};

}

#endif
