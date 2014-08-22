#include <cfloat>
#include <map>
#include <liblbfgs/lbfgs.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tune-lbfgs.h>
#include <travatar/gradient.h>
#include <travatar/global-debug.h>
#include <travatar/softmax.h>
#include <travatar/dict.h>
#include <travatar/weights.h>
#include <travatar/eval-measure.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/tuning-example.h>

using namespace std;
using namespace boost;
using namespace travatar;

TuneLbfgs::~TuneLbfgs() {
    if(gradient_)
        delete gradient_;
}

double TuneLbfgs::operator()(size_t n, const double * x, double * g) const {
    return gradient_->CalcGradient(n, x, g);
}

// Tune new weights using the expected BLEU algorithm
double TuneLbfgs::RunTuning(SparseMap & kv) {

    // Sanity checks
    if(examps_.size() < 1)
        THROW_ERROR("Must have at least one example to perform tuning");
    if(l1_coeff_ != 0.0)
        THROW_ERROR("L1 regularization is not supported yet.");

    // Start
    PRINT_DEBUG("Starting L-BFGS Tuning Run: " << Dict::PrintSparseMap(kv) << endl, 2);

    // The final score
    double last_score = 0.0;

    gradient_->SetMult(-1);
    // Initialize the weights appropriately
    vector<double> weights;
    if(!use_init_)
        kv = SparseMap();
    gradient_->DensifyWeights(kv, weights);

    // Optimize and save
    liblbfgs::LBFGS<TuneLbfgs> lbfgs(*this, iters_, l1_coeff_, 1);
    last_score = lbfgs(weights.size(), &(*weights.begin()));
    
    gradient_->SparsifyWeights(weights, kv);

    return last_score * -1;
}
