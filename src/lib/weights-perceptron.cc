#include <travatar/weights-perceptron.h>
#include <travatar/util.h>

using namespace std;
using namespace travatar;

inline double InRange(double val, pair<double,double> range) {
    return min(max(val,range.first), range.second);
}

// The pairwise weight update rule
void WeightsPerceptron::Update(
    const SparseMap & oracle, double oracle_score, double oracle_loss,
    const SparseMap & system, double system_score, double system_loss
) {
    if(system_score >= oracle_score) {
        SparseMap change = (oracle - system);
        BOOST_FOREACH(SparseMap::value_type change_val, change) {
            // Update the value
            double new_val = GetCurrent(change_val.first) + change_val.second;
            // And ensure we are in the correct range
            current_[change_val.first] = InRange(new_val, GetRange(change_val.first));
        }
    }
    curr_iter_++;
    PRINT_DEBUG(current_ << std::endl, 2);
}

// Get the current values of the weights at this point in learning
double WeightsPerceptron::GetCurrent(const SparseMap::key_type & key) {
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
