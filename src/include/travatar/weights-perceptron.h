#ifndef WEIGHTS_PERCEPTRON_H__
#define WEIGHTS_PERCEPTRON_H__

#include <boost/foreach.hpp>
#include <travatar/weights-pairwise.h>
#include <cmath>

namespace travatar {

class WeightsPerceptron : public WeightsPairwise {

public:
    WeightsPerceptron() : WeightsPairwise(), curr_iter_(0), l1_coeff_(0), rate_(1) { }
    WeightsPerceptron(const SparseMap & current) : WeightsPairwise(current), curr_iter_(0), l1_coeff_(0), rate_(1) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseMap & oracle, double oracle_model, double oracle_eval,
        const SparseMap & system, double system_model, double system_eval
    );

    // Adjust the weights according to the n-best list
    // Scores are current model scores and evaluation scores
    virtual void Adjust(
            const std::vector<std::pair<double,double> > & scores,
            const std::vector<SparseMap*> & features) {
        int oracle = 0, system = 0;
        for(int i = 1; i < (int)scores.size(); i++) {
            if(scores[i].first > scores[system].first)
                system = i;
            if(scores[i].second > scores[oracle].second)
                oracle = i;
        }
        Update(*features[oracle], scores[oracle].first, scores[oracle].second,
               *features[system], scores[system].first, scores[system].second);
    }

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
    void SetLearningRate(double rate) {
        rate_ = rate;
    }

protected:

    SparseIntMap last_update_;
    int curr_iter_;
    double l1_coeff_;
    double rate_;

};

}

#endif
