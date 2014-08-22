#ifndef TUNE_XEVAL_H__
#define TUNE_XEVAL_H__

#include <travatar/tune.h>
#include <travatar/sparse-map.h>
#include <travatar/eval-measure.h>
#include <boost/thread.hpp>
#include <tr1/unordered_map>
#include <vector>
#include <cfloat>

namespace travatar {

class Weights;
class Gradient;

// Performs gradient ascent to maximize the expectation of the eval measure
// This is similar to the methods proposed in:
//   David Smith and Jason Eisner
//   Minimum Risk Annealing for Training Log-Linear Models
//
//   Rosti, A.-V., Zhang, B., Matsoukas, S. and Schwartz, R.
//   BBN System Description for WMT10 System Combination Task
class TuneXeval : public Tune {

public:

    TuneXeval(Gradient* gradient) : iters_(100), iter_(0),
                  l1_coeff_(0.0),
                  optimizer_("lbfgs"),
                  use_init_(true), gradient_(gradient) { }

    ~TuneXeval();

    // Tune new weights to maximize the expectation of the evaluation measure
    virtual double RunTuning(SparseMap & weights);

    // For tuning with LBFGS
    double operator()(size_t n, const double * x, double * g) const;

    void SetIters(int iters) { iters_ = iters; }
    void SetL1Coefficient(double l1_coeff) { l1_coeff_ = l1_coeff; }

protected:
    int iters_;
    mutable int iter_;
    double l1_coeff_;
    std::string optimizer_;
    bool use_init_;
    Gradient* gradient_;


};

}

#endif
