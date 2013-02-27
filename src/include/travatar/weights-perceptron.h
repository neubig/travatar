#ifndef WEIGHTS_PERCEPTRON_H__
#define WEIGHTS_PERCEPTRON_H__

#include <travatar/weights-pairwise.h>
#include <cmath>

namespace travatar {

class WeightsPerceptron : public WeightsPairwise {

public:
    WeightsPerceptron() : WeightsPairwise(), curr_iter_(0), l1_coeff_(1e-5) { }
    WeightsPerceptron(const SparseMap & current) : WeightsPairwise(current), curr_iter_(0), l1_coeff_(1e-5) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseMap & oracle, double oracle_score, double oracle_loss,
        const SparseMap & system, double system_score, double system_loss
    ) {
        if(system_score >= oracle_score) {
            SparseMap change = (oracle - system);
            // std::cerr << "CHANGE=" << change << std::endl;
            BOOST_FOREACH(SparseMap::value_type change_val, change) {
                double new_val = GetCurrent(change_val.first) + change_val.second;
                // std::cerr << "BEFORE: " << Dict::WSym(change_val.first) << ", " << current_ << ", new_val=" << new_val << std::endl;
                current_[change_val.first] = new_val;
                // std::cerr << "AFTER: " << Dict::WSym(change_val.first) << ", " << current_ << std::endl;
            }
        }
        curr_iter_++;
        std::cerr << current_ << std::endl;
    }

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key) {
        // std::cerr << "GetCurrent key=" << Dict::WSym(key) << ", current_=" << current_ << std::endl;
        SparseMap::iterator it = current_.find(key);
        // Non-existant weights are zero
        if(it == current_.end())
            return 0.0;
        // Find the difference between the last update
        int diff = curr_iter_ - last_update_[key];
        if(diff != 0) {
            double reg = diff*l1_coeff_;
            // std::cerr << "key=" << Dict::WSym(key) << ", reg=" << reg << ", it-second=" << it->second << ", fabs(it-second)=" << fabs(it->second) << std::endl;
            if(fabs(it->second) < reg) {
                // std::cerr << " ERASED" << std::endl;
                current_.erase(it);
                return 0.0;
            } else if (it->second > 0) {
                it->second -= reg;
            } else {
                it->second += reg;
            }
            last_update_[key] = curr_iter_;
        }
        // std::cerr << " RET: " << it->second << std::endl;
        return it->second;
    }

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        BOOST_FOREACH(SparseMap::value_type val, current_)
            val.second = GetCurrent(val.first);
        return current_;
    }

protected:

    SparseIntMap last_update_;
    int curr_iter_;
    double l1_coeff_;

};

}

#endif
