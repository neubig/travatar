#ifndef WEIGHTS_AVERAGE_PERCEPTRON_H__
#define WEIGHTS_AVERAGE_PERCEPTRON_H__

#include <travatar/weights-perceptron.h>
#include <travatar/sparse-map.h>
#include <cmath>

namespace travatar {

class WeightsAveragePerceptron : public WeightsPerceptron {

public:
    WeightsAveragePerceptron() : WeightsPerceptron() { }
    WeightsAveragePerceptron(const SparseMap & current) : WeightsPerceptron(current) { }

    // Update the weights
    virtual void Update(
        const SparseMap & oracle, double oracle_model, double oracle_eval,
        const SparseMap & system, double system_model, double system_eval
    );

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key);
    virtual double GetCurrent(const SparseMap::key_type & key) const;

    // Get the final values of the weights
    virtual const SparseMap & GetFinal();

protected:
    
    SparseIntMap last_update_;
    SparseMap final_;

};

}

#endif
