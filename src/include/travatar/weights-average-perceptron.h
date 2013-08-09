#ifndef WEIGHTS_AVERAGE_PERCEPTRON_H__
#define WEIGHTS_AVERAGE_PERCEPTRON_H__

#include <boost/foreach.hpp>
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
    virtual const SparseMap & GetFinal() {
        if(curr_iter_) {
            BOOST_FOREACH(SparseMap::value_type final_val, current_) {
                // Save the old value and get the new value in the range
                int prev_iter = last_update_[final_val.first];
                double avg_val = final_[final_val.first];
                double new_val = GetCurrent(final_val.first);
                final_[final_val.first] = (avg_val*prev_iter + new_val*(curr_iter_-prev_iter))/curr_iter_;
                last_update_[final_val.first] = curr_iter_;
            }
            return final_;
        } else {
            return current_;
        }
    }

protected:
    
    SparseIntMap last_update_;
    SparseMap final_;

};

}

#endif
