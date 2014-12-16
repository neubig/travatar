#include <travatar/weights-adagrad.h>
#include <travatar/global-debug.h>

using namespace std;
using namespace travatar;

// The pairwise weight update rule
void WeightsAdagrad::Update(
    const SparseVector & oracle, Real oracle_score, Real oracle_eval,
    const SparseVector & system, Real system_score, Real system_eval
) {
    curr_iter_++;
    if(system_score + (oracle_eval-system_eval)*margin_scale_ >= oracle_score) {
        SparseVector change = oracle - system;
        BOOST_FOREACH(const SparsePair & change_val, change.GetImpl()) {
            if(change_val.second == 0) continue;
            Real new_val = GetCurrent(change_val.first);
            varinv_[change_val.first] += change_val.second * change_val.second;
            new_val += rate_ * sqrt(1/varinv_[change_val.first]) * change_val.second;
            current_[change_val.first] = new_val;
        }
    }
    PRINT_DEBUG(current_ << std::endl, 2);
}
