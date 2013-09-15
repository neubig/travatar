#include <cfloat>
#include <map>
#include <liblbfgs/lbfgs.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tune-xeval.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/weights.h>
#include <travatar/eval-measure.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/tuning-example.h>

using namespace std;
using namespace boost;
using namespace travatar;

inline double DivideZero(double x, double y) {
    return (y==0?0:x/y);
}

void TuneXeval::CalcAvgGradient(
            const vector<vector<double> > & p_i_k,
            const vector<vector<EvalStatsPtr> > & stats_i_k,
            const vector<EvalStatsPtr> & stats_i,
            const EvalStatsPtr & stats, 
            const Weights & weights,
            SparseMap & d_xeval_dw) const {
    EvalStatsAverage * stats_avg = (EvalStatsAverage*)stats.get();
    int N = p_i_k.size();
    // Calculate the stats for each example
    for(int i = 0; i < N; i++) {
        int K = p_i_k[i].size();
        // The amount to multiply each member k' by
        vector<double> d_xeval_dsikprime(K,0);
        for(int k = 0; k < K; k++) {
            EvalStatsAverage * stats_i_k_avg = (EvalStatsAverage*)stats_i_k[i][k].get();
            PRINT_DEBUG("Stats i=" << i << ", k=" << k << ": " << stats_i_k_avg->ConvertToString() << endl, 3);
            // Calculate the derivative of xeval
            double d_xeval_logpik = stats_i_k_avg->GetVal()/stats_avg->GetDenom();
            // Now multiply this value for every k'
            for(int kprime = 0; kprime < K; kprime++)
                d_xeval_dsikprime[kprime] += 
                    d_xeval_logpik * ((kprime == k?1:0)-p_i_k[i][kprime]);
        }
        // Calculate the actual gradient
        const vector<ExamplePair> & nbest = examps_[i]->CalculateNbest(weights);
        for(int kprime = 0; kprime < K; kprime++)
            d_xeval_dw += nbest[kprime].first * d_xeval_dsikprime[kprime];
    }
}

void TuneXeval::CalcBleuGradient(
            const vector<vector<double> > & p_i_k,
            const vector<vector<EvalStatsPtr> > & stats_i_k,
            const vector<EvalStatsPtr> & stats_i,
            const EvalStatsPtr & stats, 
            const Weights & weights,
            SparseMap & d_xeval_dw) const {
    // Overall stats
    EvalStatsBleu * stats_bleu = (EvalStatsBleu*)stats.get();
    int N = p_i_k.size();
    double P = stats_bleu->GetAvgLogPrecision();
    double eP = exp(P);
    double R = 1.0/stats_bleu->GetLengthRatio();
    double omR = 1-R;
    double eomR = exp(omR), e10komR = std::min(DBL_MAX,exp(10000*omR));
    double B = (eomR-1)/(e10komR+1) + 1;
    // This is used in the calculation of dB/log(p_{i,k})
    double d_B_logpik_Rpart = (eomR/(e10komR+1)-e10komR/(e10komR+1)/(e10komR+1)*10000*(eomR-1)) * -R;
    PRINT_DEBUG("P=" << P << ", B=" << B << ", R=" << R << ", Rpart=" << d_B_logpik_Rpart << ", e10komR="<<e10komR<<", eomR=" << eomR << endl, 2);

    // Calculate the stats for each example
    for(int i = 0; i < N; i++) {
        int K = p_i_k[i].size();
        EvalStatsBleu * stats_i_bleu = (EvalStatsBleu*)stats_i[i].get();
        // The amount to multiply each member k' by
        vector<double> d_xeval_dsikprime(K,0);
        for(int k = 0; k < K; k++) {
            EvalStatsBleu * stats_i_k_bleu = (EvalStatsBleu*)stats_i_k[i][k].get();
            PRINT_DEBUG("s_i_k_bleu: " << stats_i_k_bleu->ConvertToString() << endl, 2);
            // Calculate the derivative of exp(P) with respect to log(p_{i,k})
            double d_expP_logpik = 0;
            for(int n = 0; n < stats_bleu->GetNgramOrder(); n++)
                d_expP_logpik += DivideZero(stats_i_k_bleu->GetMatch(n),stats_i_bleu->GetMatch(n)) -
                                 DivideZero(stats_i_k_bleu->GetCount(n),stats_i_bleu->GetCount(n));
            d_expP_logpik /= stats_bleu->GetNgramOrder();
            PRINT_DEBUG("d_expP_logpik: " << d_expP_logpik << endl, 2);
            // Calculate the derivative of B with respect to log(p_{i,k})
            double d_B_logpik = d_B_logpik_Rpart * (
                                    DivideZero(stats_i_k_bleu->GetRefLen(),stats_i_bleu->GetRefLen()) -
                                    DivideZero(stats_i_k_bleu->GetCount(1),stats_i_bleu->GetCount(1)));
            PRINT_DEBUG("d_B_logpik: " << d_B_logpik << endl, 2);
            // Calculate the derivative of xeval
            double d_xeval_logpik = eP * ( d_B_logpik + d_expP_logpik * B );
            PRINT_DEBUG("d_xeval_logpik: " << d_xeval_logpik << endl, 2);
            // Now multiply this value for every k'
            for(int kprime = 0; kprime < K; kprime++)
                d_xeval_dsikprime[kprime] += 
                    d_xeval_logpik * ((kprime == k?1:0)-p_i_k[i][kprime]);
        }
        // Calculate the actual gradient
        const vector<ExamplePair> & nbest = examps_[i]->CalculateNbest(weights);
        for(int kprime = 0; kprime < K; kprime++)
            d_xeval_dw += nbest[kprime].first * d_xeval_dsikprime[kprime]; 
        PRINT_DEBUG("s_xeval_dw: " << Dict::PrintFeatures(d_xeval_dw) << endl, 2);
    }
}

void TuneXeval::Init() {
    // If we have no sparse-to-dense map
    if(sparse2dense_.size() == 0) {
        SparseMap potential;
        BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_)
            examp->CountWeights(potential);
        BOOST_FOREACH(SparseMap::value_type val, potential) {
            sparse2dense_[val.first] = dense2sparse_.size();
            dense2sparse_.push_back(val.first);
        }
    }
}

double TuneXeval::operator()(size_t n, const double * x, double * g) const {
    SparseMap kv, d_xeval_dw;
    for(size_t i = 0; i < n; i++) {
        kv[dense2sparse_[i]] = x[i];
        g[i] = 0;
    }
    double ret = CalcGradient(kv, d_xeval_dw);
    BOOST_FOREACH(SparseMap::value_type val, d_xeval_dw) {
        SparseIntMap::const_iterator it = sparse2dense_.find(val.first);
        if(it == sparse2dense_.end())
            THROW_ERROR("Could not find feature in sparse-to-dense mapping");
        g[it->second] = val.second;
    }
    return ret;
}

double TuneXeval::CalcGradient(const SparseMap & kv, SparseMap & d_xeval_dw) const {

    iter_++;

    // Allocate some space for statistics
    shared_ptr<Weights> weights(new Weights(kv));
    EvalStatsPtr first_stats = examps_[0]->CalculateNbest(*weights)[0].second;
    int N = examps_.size();
    vector<vector<double> > p_i_k(N);
    vector<vector<EvalStatsPtr> > stats_i_k(N);
    vector<EvalStatsPtr> stats_i(N); 
    EvalStatsPtr stats;
    // Create the expected stats, make sure it is of the right type by cloning
    // the first stats of the first n-best list
    stats = first_stats->Times(0);

    // ***** Calculate the expectation of the statistics *****
    for(int i = 0; i < N; i++) {
        shared_ptr<TuningExample> examp = examps_[i];

        // Get the n-best list
        const vector<ExamplePair> & nbest = examp->CalculateNbest(*weights);

        // Get the probabilities of the n-best list
        if(p_i_k[i].size()!=nbest.size()) p_i_k[i].resize(nbest.size());
        for(int k = 0; k < (int)nbest.size(); k++)
            p_i_k[i][k] = (*weights) * nbest[k].first;
        p_i_k[i] = Softmax(p_i_k[i]);

        // Add to the expectation of the statistics
        if(stats_i_k[i].size() != nbest.size()) stats_i_k[i].resize(nbest.size());
        stats_i[i] = first_stats->Times(0);
        for(int k = 0; k < (int)nbest.size(); k++) {
            PRINT_DEBUG("Iter " << iter_ << " "<<i<<" " << k << ": p=" << p_i_k[i][k] << endl, 3);
            stats_i_k[i][k] = nbest[k].second->Times(p_i_k[i][k]);
            stats_i[i]->PlusEquals(*stats_i_k[i][k]);
        }
        stats->PlusEquals(*stats_i[i]);
        PRINT_DEBUG("Iter " << iter_ << " "<<i<<": F=" << p_i_k[i][0] << " S=" << stats_i[i]->ConvertToString() << endl, 2);
    }
    
    if(first_stats->GetIdString() == "BLEU") {
        CalcBleuGradient(p_i_k, stats_i_k, stats_i, stats, *weights, d_xeval_dw);
    } else if(first_stats->GetIdString() == "AVG" || first_stats->GetIdString() == "RIBES") {
        CalcAvgGradient(p_i_k, stats_i_k, stats_i, stats, *weights, d_xeval_dw);
    } else {
        THROW_ERROR("Cannot optimize expectation of "<<first_stats->GetIdString()<<" yet");
    }

    double score = stats->ConvertToScore();

    // Perform L2 regularization if necessary
    if(l2_coeff_ != 0.0) {
        BOOST_FOREACH(SparseMap::value_type val, weights->GetCurrent()) {
            if(val.second != 0.0) {
                score -= l2_coeff_*val.second*val.second;
                d_xeval_dw[val.first] -= 2 * l2_coeff_ * val.second;
            }
        }
    }

    // If there is a multiplier to the gradient (i.e. -1)
    if(mult_ != 1.0)
        d_xeval_dw = d_xeval_dw * mult_;

    PRINT_DEBUG("Iter " << iter_ << " Xeval: " << score << " @ " << stats->ConvertToString() << endl, 1);

    return score * mult_;
}

// Tune new weights using the expected BLEU algorithm
double TuneXeval::RunTuning(SparseMap & kv) {

    // Always start from zero
    kv = SparseMap();

    // Sanity checks
    if(examps_.size() < 1)
        THROW_ERROR("Must have at least one example to perform tuning");

    // Start
    PRINT_DEBUG("Starting Expected Eval Tuning Run: " << Dict::PrintFeatures(kv) << endl, 2);

    // The final score
    double last_score = 0.0;

    // Do iterations of SGD learning
    if(optimizer_ == "sgd") {
        if(l1_coeff_ != 0.0)
            THROW_ERROR("L1 regularization and SGD are not compatible yet.");
        double step_size_ = 1.0;
        for(int iter = 0; iter < iters_; iter++) {
            // Calculate the gradient and score
            SparseMap d_xeval_dw;
            last_score = CalcGradient(kv, d_xeval_dw);

            // Perform stochastic gradient descent
            kv += d_xeval_dw * step_size_;

            PRINT_DEBUG("Iter " << iter+1 << " KV: " << Dict::PrintFeatures(kv) << endl, 2);
        }
    // Optimize using LBFGS. Iterations are done within the library
    } else if (optimizer_ == "lbfgs") {
        mult_ = -1;
        liblbfgs::LBFGS<TuneXeval> lbfgs(*this, iters_, l1_coeff_, 1);
        vector<double> weights(dense2sparse_.size(),0.0);
        last_score = lbfgs(weights.size(), &(*weights.begin()));
        kv = SparseMap();
        for(int i = 0; i < (int)dense2sparse_.size(); i++)
            if(weights[i])
                kv[dense2sparse_[i]] = weights[i];
    }

    return last_score * mult_;
}
