#include <travatar/weights-average-perceptron.h>
#include <travatar/util.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

inline double InRange(double val, pair<double,double> range) {
    return min(max(val,range.first), range.second);
}

// Get the final values of the weights
const SparseMap & WeightsAveragePerceptron::GetFinal() {
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

// The pairwise weight update rule
void WeightsAveragePerceptron::Update(
    const SparseMap & oracle, double oracle_model, double oracle_eval,
    const SparseMap & system, double system_model, double system_eval) {
    ++curr_iter_;
    if(system_eval < oracle_eval) {
        if(l1_coeff_ != 0) THROW_ERROR("Non-zero regularization in averaged perceptron not accounted for yet");
        SparseMap change = (oracle - system);
        PRINT_DEBUG("ORACLE: " << Dict::PrintSparseMap(oracle) << endl << 
                    "SYSTEM: " << Dict::PrintSparseMap(system) << endl << 
                    "CHANGE: " << Dict::PrintSparseMap(change) << endl, 4);
        BOOST_FOREACH(SparseMap::value_type change_val, change) {
            // Save the old value and get the new value in the range
            int prev_iter = last_update_[change_val.first];
            double avg_val = final_[change_val.first];
            double old_val = GetCurrent(change_val.first);
            double new_val = InRange(old_val + change_val.second, GetRange(change_val.first));
            current_[change_val.first] = new_val;
            // The new average is equal to (avg_val*prev_iter + old_val*(curr_iter_-prev_iter-1)+new_val)/curr_iter_;
            final_[change_val.first] = (avg_val*prev_iter + old_val*(curr_iter_-prev_iter-1)+new_val)/curr_iter_;
            last_update_[change_val.first] = curr_iter_;
        }
    }
    PRINT_DEBUG(current_ << std::endl, 2);
}

// Not currently implemented. Will need to add the weights as volataile and locks.
double WeightsAveragePerceptron::GetCurrent(const SparseMap::key_type & key) const {
    THROW_ERROR("Constant access to perceptron weights is not currently supported");
    return 0.0;
}
// Get the current values of the weights at this point in learning
double WeightsAveragePerceptron::GetCurrent(const SparseMap::key_type & key) {
    SparseMap::iterator it = current_.find(key);
    // Non-existant weights are zero
    if(it == current_.end())
        return 0.0;
    // Find the difference between the last update
    int diff = curr_iter_ - last_update_[key];
    if(diff != 0) {
        double reg = diff*l1_coeff_;
        // Find the value
        if(it->second > 0)
            it->second = max(it->second-reg,0.0);
        else
            it->second = min(it->second+reg,0.0);
        it->second = InRange(it->second, GetRange(key));
        last_update_[key] = curr_iter_;
        if(it->second == 0) {
            current_.erase(it);
            return 0.0;
        }
    }
    // std::cerr << " RET: " << it->second << std::endl;
    return it->second;
}
