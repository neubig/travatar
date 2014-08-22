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

// Tune using the L-BFGS algorithm
class TuneLbfgs : public Tune {

public:

    TuneLbfgs(Gradient* gradient) : iters_(100), iter_(0),
                  l1_coeff_(0.0),
                  use_init_(true), gradient_(gradient) { }

    ~TuneLbfgs();

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
