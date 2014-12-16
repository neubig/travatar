#ifndef WEIGHTS_PERCEPTRON_H__
#define WEIGHTS_PERCEPTRON_H__

#include <travatar/real.h>
#include <travatar/weights-pairwise.h>
#include <cmath>

namespace travatar {

class WeightsPerceptron : public WeightsPairwise {

public:
    WeightsPerceptron() : WeightsPairwise(), curr_iter_(0), l1_coeff_(0), rate_(1), margin_scale_(0) { }
    WeightsPerceptron(const SparseMap & current) : WeightsPairwise(current), curr_iter_(0), l1_coeff_(0), rate_(1), margin_scale_(0) { }

    // The pairwise weight update rule
    virtual void Update(
        const SparseVector & oracle, Real oracle_model, Real oracle_eval,
        const SparseVector & system, Real system_model, Real system_eval
    );

    // Adjust the weights according to the n-best list
    // Scores are current model scores and evaluation scores
    virtual void AdjustNbest(
            const std::vector<std::pair<Real,Real> > & scores,
            const std::vector<SparseVector*> & features) {
        int oracle = 0, system = 0;
        Real best_score = -REAL_MAX;
        for(int i = 0; i < (int)scores.size(); i++) {
            Real margin_score = scores[i].first - scores[i].second * margin_scale_;
            if(margin_score > best_score) {
                system = i;
                best_score = margin_score;
            }
            if(scores[i].second > scores[oracle].second)
                oracle = i;
        }
        Update(*features[oracle], scores[oracle].first, scores[oracle].second,
               *features[system], scores[system].first, scores[system].second);
    }

    // Get the current values of the weights at this point in learning
    virtual Real GetCurrent(const SparseMap::key_type & key);
    virtual Real GetCurrent(const SparseMap::key_type & key) const;

    // Get the final values of the weights
    virtual const SparseMap & GetFinal();

    void SetL1Coeff(Real l1_coeff) { l1_coeff_ = l1_coeff; }
    void SetLearningRate(Real rate) { rate_ = rate; }
    void SetMarginScale(Real margin) { margin_scale_ = margin; }

protected:

    SparseIntMap last_update_;
    int curr_iter_;
    Real l1_coeff_;
    Real rate_;
    Real margin_scale_;

};

}

#endif
