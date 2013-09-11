#ifndef TUNE_XEVAL_H__
#define TUNE_XEVAL_H__

#include <vector>
#include <cfloat>
#include <boost/thread.hpp>
#include <tr1/unordered_map>
#include <travatar/sparse-map.h>
#include <travatar/tune.h>
#include <travatar/eval-measure.h>

namespace travatar {

class Weights;

// Performs gradient ascent to maximize the expectation of the eval measure
// This is a generalization of the method proposed in:
//   Rosti, A.-V., Zhang, B., Matsoukas, S. and Schwartz, R.
//   BBN System Description for WMT10 System Combination Task
//
// The implementation here follows the description (in Japanese)
//   機械翻訳 7.2.4章 (ベイズリスク最小化)
class TuneXeval : public Tune {

public:

    TuneXeval() : iters_(100), iter_(0), optimizer_("sgd") { }

    // Tune new weights to maximize the expectation of the evaluation measure
    virtual double RunTuning(SparseMap & weights);

    // Calculate the gradient for particular weights
    // The return is the expectation of the evaluation
    double CalcGradient(const SparseMap & weights, SparseMap & d_xeval_dw);

    // Calculate the gradient for averaged measures based on expectations, probabilities
    void CalcAvgGradient(
            const std::vector<std::vector<double> > & p_i_k,
            const std::vector<std::vector<EvalStatsPtr> > & stats_i_k,
            const std::vector<EvalStatsPtr> & stats_i,
            const EvalStatsPtr & stats, 
            const Weights & weights,
            SparseMap & d_xeval_dw);
    
    // Calculate the gradient for BLEU based on expectations, probabilities
    void CalcBleuGradient(
            const std::vector<std::vector<double> > & p_i_k,
            const std::vector<std::vector<EvalStatsPtr> > & stats_i_k,
            const std::vector<EvalStatsPtr> & stats_i,
            const EvalStatsPtr & stats, 
            const Weights & weights,
            SparseMap & d_xeval_dw);

    void SetIters(int iters) { iters_ = iters; }

protected:
    int iters_, iter_;
    std::string optimizer_;
    

};

}

#endif
