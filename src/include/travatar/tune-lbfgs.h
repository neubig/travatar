#ifndef TUNE_XEVAL_H__
#define TUNE_XEVAL_H__

#include <travatar/real.h>
#include <travatar/tune.h>
#include <travatar/sparse-map.h>
#include <travatar/eval-measure.h>
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

    // Initialize the data structures
    virtual void Init(const SparseMap & init_weights);

    // Tune new weights to maximize the expectation of the evaluation measure
    virtual Real RunTuning(SparseMap & weights);

    // For tuning with LBFGS
    Real operator()(size_t n, const Real * x, Real * g) const;

    void SetIters(int iters) { iters_ = iters; }
    void SetL1Coefficient(Real l1_coeff) { l1_coeff_ = l1_coeff; }

protected:
    int iters_;
    mutable int iter_;
    Real l1_coeff_;
    std::string optimizer_;
    bool use_init_;
    Gradient* gradient_;


};

}

#endif
