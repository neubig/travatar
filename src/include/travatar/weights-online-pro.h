#ifndef WEIGHTS_ONLINE_PRO_H__
#define WEIGHTS_ONLINE_PRO_H__

#include <travatar/weights.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

class WeightsOnlinePro : public Weights {

public:

    WeightsOnlinePro(const boost::shared_ptr<Weights> & weights)
        : weights_(weights), num_samples_(5000), num_updates_(50),
          diff_threshold_(0.05), alpha_scale_(1.0) { }

    // Adjust the weights according to the n-best list
    void SetAlphaScale(double alpha_scale) { alpha_scale_ = alpha_scale; }

    // Adjust the weights according to the n-best list
    // Scores are current model scores and evaluation scores
    virtual void Adjust(
            const std::vector<std::pair<double,double> > & scores,
            const std::vector<SparseMap*> & features);

    virtual const SparseMap & GetCurrent() const { return weights_->GetCurrent(); }
    virtual const SparseMap & GetFinal() const { return weights_->GetFinal(); }
    virtual void SetCurrent(const SparseMap & weights) { weights_->SetCurrent(weights); }

protected:
    boost::shared_ptr<Weights> weights_;
    int num_samples_;
    int num_updates_;
    double diff_threshold_;
    double alpha_scale_;

};

}

#endif
