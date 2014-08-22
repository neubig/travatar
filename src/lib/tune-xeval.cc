#include <cfloat>
#include <map>
#include <liblbfgs/lbfgs.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tune-xeval.h>
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

TuneXeval::~TuneXeval() {
    if(gradient_)
        delete gradient_;
}

double TuneXeval::operator()(size_t n, const double * x, double * g) const {
    return gradient_->CalcGradient(n, x, g);
}

// Tune new weights using the expected BLEU algorithm
double TuneXeval::RunTuning(SparseMap & kv) {

    // Sanity checks
    if(examps_.size() < 1)
        THROW_ERROR("Must have at least one example to perform tuning");
    if(l1_coeff_ != 0.0)
        THROW_ERROR("L1 regularization is not supported yet.");

    // Start
    PRINT_DEBUG("Starting Expected Eval Tuning Run: " << Dict::PrintSparseMap(kv) << endl, 2);

    // The final score
    double last_score = 0.0;

    // Do iterations of SGD learning
    if(optimizer_ == "sgd") {
        double step_size_ = 1.0;
        for(int iter = 0; iter < iters_; iter++) {
            // Calculate the gradient and score
            SparseMap d_xeval_dw;
            last_score = gradient_->CalcSparseGradient(kv, d_xeval_dw);

            // Perform stochastic gradient descent
            kv += d_xeval_dw * step_size_;

            PRINT_DEBUG("Iter " << iter+1 << " KV: " << Dict::PrintSparseMap(kv) << endl, 2);
        }
    // Optimize using LBFGS. Iterations are done within the library
    } else if (optimizer_ == "lbfgs") {
        gradient_->SetMult(-1);
        // Initialize the weights appropriately
        vector<double> weights;
        if(!use_init_)
            kv = SparseMap();
        gradient_->DensifyWeights(kv, weights);

        // Optimize and save
        liblbfgs::LBFGS<TuneXeval> lbfgs(*this, iters_, l1_coeff_, 1);
        last_score = lbfgs(weights.size(), &(*weights.begin()));
        
        gradient_->SparsifyWeights(weights, kv);

    }

    return last_score * gradient_->GetMult();
}
