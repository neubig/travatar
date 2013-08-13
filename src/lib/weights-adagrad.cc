#include <travatar/weights-adagrad.h>
#include <travatar/util.h>

using namespace std;
using namespace travatar;

// The pairwise weight update rule
void WeightsAdagrad::Update(
    const SparseMap & oracle, double oracle_score, double oracle_eval,
    const SparseMap & system, double system_score, double system_eval
) {
    curr_iter_++;
    if(system_score >= oracle_score) {
        SparseMap change = (oracle - system) * (oracle_eval - system_eval);
        BOOST_FOREACH(SparseMap::value_type change_val, change) {
            if(change_val.second == 0) continue;
            double new_val = GetCurrent(change_val.first);
            varinv_[change_val.first] += change_val.second * change_val.second;
            new_val += rate_ * sqrt(1/varinv_[change_val.first]) * change_val.second;
            current_[change_val.first] = new_val;
        }
    }
    PRINT_DEBUG(current_ << std::endl, 2);
}
