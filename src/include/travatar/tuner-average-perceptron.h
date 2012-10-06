#ifndef TUNER_AVERAGE_PERCEPTRON_H__
#define TUNER_AVERAGE_PERCEPTRON_H__

#include <travatar/tuner-pairwise.h>

namespace travatar {

// A virtual parent class for tuning strategies
class TunerAveragePerceptron : public TunerPairwise {

public:
    
    TunerAveragePerceptron() : step_count_(0) { }
    
    // Do a pairwise comparison between SparseMaps and update if necessary
    virtual void PairwiseUpdate(
            const SparseMap & oracle_feat, double oracle_score,
            const SparseMap & system_feat, double system_score,
            SparseMap & weights) {
        step_count_++;
        // Update if we made a mistake
        if(system_score >= oracle_score) {
            SparseMap change = oracle_feat - system_feat;
            BOOST_FOREACH(const SparseMap::value_type val, change) {
                if(val.second == 0) continue;
                int last = last_updated_[val.first];
                // Update the average value
                avg_weights_[val.first] = (
                    avg_weights_[val.first]*last + // Average component
                    weights[val.first]*(step_count_-last-1) + // Recent
                    (weights[val.first]+val.second) // Current
                    ) / step_count_;
                // Update the current value
                weights[val.first] += val.second;
                // Update the last updated
                last_updated_[val.first] = step_count_;
            }
        }
    }

    void SetStepCount(int step_count) { step_count_ = step_count; }
    int GetStepCount() const { return step_count_; }

    // Update all averaged weights so that their last_updated_ step_count_
    void UpdateAllAvgWeights(const SparseMap & val) {
        THROW_ERROR("UpdateAllWeights not implemented yet");    
    }

    virtual SparseMap & GetWeights(SparseMap & def) {
        UpdateAllAvgWeights(def);
        return avg_weights_;
    }

protected:
    int step_count_;
    double step_size_;
    SparseMap avg_weights_;
    SparseIntMap last_updated_;

};

}

#endif
