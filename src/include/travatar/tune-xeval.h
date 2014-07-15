#ifndef TUNE_XEVAL_H__
#define TUNE_XEVAL_H__

#include <travatar/tune.h>
#include <travatar/sparse-map.h>
#include <travatar/dict.h>
#include <travatar/eval-measure.h>
#include <boost/thread.hpp>
#include <tr1/unordered_map>
#include <vector>
#include <cfloat>

namespace travatar {

class Weights;

// Performs gradient ascent to maximize the expectation of the eval measure
// This is similar to the methods proposed in:
//   David Smith and Jason Eisner
//   Minimum Risk Annealing for Training Log-Linear Models
//
//   Rosti, A.-V., Zhang, B., Matsoukas, S. and Schwartz, R.
//   BBN System Description for WMT10 System Combination Task
class TuneXeval : public Tune {

public:

    TuneXeval() : iters_(100), iter_(0), mult_(1.0),
                  l1_coeff_(0.0), l2_coeff_(0.0), ent_coeff_(0.0),
                  optimizer_("lbfgs"),
                  auto_scale_(false), use_init_(true),
                  dense_scale_id_(-1) { }

    // Tune new weights to maximize the expectation of the evaluation measure
    virtual double RunTuning(SparseMap & weights);

    // Calculate the gradient for particular weights
    // The return is the expectation of the evaluation
    double CalcGradient(const SparseMap & weights, SparseMap & d_xeval_dw) const;
    double CalcGradient(size_t n, const double * x, double * g) const;

    // Calculate the gradient for averaged measures based on expectations, probabilities
    void CalcAvgGradient(
            const std::vector<std::vector<double> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const double * x, double * g) const;
    
    // Calculate the gradient for BLEU based on expectations, probabilities
    void CalcBleuGradient(
            const std::vector<std::vector<double> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const double * x, double * g) const;

    // For tuning with LBFGS
    double operator()(size_t n, const double * x, double * g) const;

    // Initialize
    virtual void Init();

    void SetIters(int iters) { iters_ = iters; }
    void SetL1Coefficient(double l1_coeff) { l1_coeff_ = l1_coeff; }
    void SetL2Coefficient(double l2_coeff) { l2_coeff_ = l2_coeff; }
    void SetEntCoefficient(double ent_coeff) { ent_coeff_ = ent_coeff; }
    void SetAutoScale(bool auto_scale) { auto_scale_ = auto_scale; }

protected:
    int iters_;
    mutable int iter_;
    double mult_;
    double l1_coeff_, l2_coeff_, ent_coeff_;
    std::string optimizer_;
    std::vector<int> dense2sparse_;
    SparseIntMap sparse2dense_;
    bool auto_scale_;
    bool use_init_;
    std::vector<std::vector<EvalStatsPtr> > all_stats_;    
    std::vector<std::vector<std::vector<std::pair<WordId,double> > > > all_feats_;    
    WordId dense_scale_id_;

};

}

#endif
