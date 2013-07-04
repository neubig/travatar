#ifndef WEIGHTS_PERCEPTRON_H__
#define WEIGHTS_PERCEPTRON_H__

#include <boost/foreach.hpp>
#include <travatar/weights-pairwise.h>
#include <cmath>

namespace travatar {

class WeightsPerceptron : public WeightsPairwise {

public:
    WeightsPerceptron() : WeightsPairwise(), curr_iter_(0), l1_coeff_(0) { }
    WeightsPerceptron(const SparseMap & current) : WeightsPairwise(current), curr_iter_(0), l1_coeff_(0) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseMap & oracle, double oracle_score, double oracle_loss,
        const SparseMap & system, double system_score, double system_loss
    );

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key);
    virtual double GetCurrent(const SparseMap::key_type & key) const;

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        BOOST_FOREACH(SparseMap::value_type val, current_)
            val.second = GetCurrent(val.first);
        return current_;
    }

    void SetL1Coeff(double l1_coeff) {
        l1_coeff_ = l1_coeff;
    }

protected:

    SparseIntMap last_update_;
    int curr_iter_;
    double l1_coeff_;

};

}

#endif
