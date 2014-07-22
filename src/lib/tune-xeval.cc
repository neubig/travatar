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
inline double LogZero(double x) {
    return (x<=0?-DBL_MAX:log(x));
}

void TuneXeval::CalcAvgGradient(
            const vector<vector<double> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const double * x, double * g) const {
    EvalStatsAverage * stats_avg = (EvalStatsAverage*)stats.get();
    int N = p_i_k.size();
    // The scaling constant and its
    double gamma = (dense_scale_id_ == -1 ? 1.0 : x[dense_scale_id_]);
    if(gamma == 0.0) gamma = 1.0;
    double d_xeval_dgamma = 0;

    // Calculate the stats for each example
    for(int i = 0; i < N; i++) {
        int K = p_i_k[i].size();
        // The amount to multiply each member k' by
        vector<double> d_xeval_dsikprime(K,0);
        for(int k = 0; k < K; k++) {
            EvalStatsAverage * stats_i_k_avg = (EvalStatsAverage*)all_stats_[i][k].get();
            PRINT_DEBUG("Stats i=" << i << ", k=" << k << ": " << stats_i_k_avg->ConvertToString() << endl, 3);
            // Calculate the derivative of xeval
            double d_xeval_logpik = p_i_k[i][k]*stats_i_k_avg->GetVal()/stats_avg->GetDenom();
            // Now multiply this value for every k'
            for(int kprime = 0; kprime < K; kprime++)
                d_xeval_dsikprime[kprime] += 
                    d_xeval_logpik * ((kprime == k?1:0)-p_i_k[i][kprime]);
        }
        // Calculate the actual gradient
        for(int kprime = 0; kprime < K; kprime++) {
            double diff = d_xeval_dsikprime[kprime];
            double diffgamma = diff*gamma;
            BOOST_FOREACH(SparseMap::value_type val, all_feats_[i][kprime]) {
                g[val.first] += val.second * diffgamma;
                d_xeval_dgamma += val.second * x[val.first] * diff;
                // d_xeval_dw += nbest[kprime].first * (d_xeval_dsikprime[kprime] * gamma);
                // d_xeval_dgamma += weights * nbest[kprime].first * d_xeval_dsikprime[kprime];
            }
        }
    }
    if(auto_scale_ && d_xeval_dgamma != 0.0 && dense_scale_id_ != -1)
        g[dense_scale_id_] = d_xeval_dgamma;
}

void TuneXeval::CalcBleuGradient(
            const vector<vector<double> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const double * x, double * g) const {
    // Overall stats
    EvalStatsBleu * stats_bleu = (EvalStatsBleu*)stats.get();
    int N = p_i_k.size();
    double P = stats_bleu->GetAvgLogPrecision();
    double eP = exp(P);
    double R = 1.0/stats_bleu->GetLengthRatio();
    double mR = 1-R;
    double emR = exp(mR), e10kmR = std::min(DBL_MAX,exp(10000*mR));
    double B = (emR-1)/(e10kmR+1) + 1;
    double Bprime = (emR-(emR-1)*10000*(e10kmR/(1+e10kmR)))/(1+e10kmR);
    // This is used in the calculation of dB/log(p_{i,k})
    PRINT_DEBUG("P=" << P << ", eP=" << eP << ", B=" << B << ", Bprime=" << Bprime << ", R=" << R << ", e10kmR="<<e10kmR<<", emR=" << emR << endl, 2);
    // The left and right constants
    double left = eP * Bprime * -R;
    double right = eP * B / stats_bleu->GetNgramOrder();
    // The scaling constant and its
    double gamma = (dense_scale_id_ == -1 ? 1.0 : x[dense_scale_id_]);
    if(gamma == 0.0) gamma = 1.0;
    double d_xeval_dgamma = 0;

    // Calculate the stats for each example
    for(int i = 0; i < N; i++) {
        int K = p_i_k[i].size();
        // The amount to multiply each member k' by
        vector<double> d_xeval_dsikprime(K,0);
        for(int k = 0; k < K; k++) {
            EvalStatsBleu * stats_i_k_bleu = (EvalStatsBleu*)all_stats_[i][k].get();
            PRINT_DEBUG("s_i_k_bleu: " << stats_i_k_bleu->ConvertToString() << endl, 2);
            double pik = p_i_k[i][k];
            // Calculate the derivative of exp(P) with respect to log(p_{i,k})
            double my_right = 0;
            for(int n = 0; n < stats_bleu->GetNgramOrder(); n++)
                my_right += DivideZero(stats_i_k_bleu->GetMatch(n)*pik,stats_bleu->GetMatch(n)) -
                            DivideZero(stats_i_k_bleu->GetSysCount(n)*pik,stats_bleu->GetSysCount(n));
            my_right *= right;
            PRINT_DEBUG("my_right: " << my_right << endl, 2);
            // Calculate the derivative of B with respect to log(p_{i,k})
            double my_left = left * (
                             DivideZero(stats_i_k_bleu->GetRefCount(0)*pik,stats_bleu->GetRefCount(0)) -
                             DivideZero(stats_i_k_bleu->GetSysCount(0)*pik,stats_bleu->GetSysCount(0)));
            PRINT_DEBUG("my_left: " << my_left << endl, 2);
            // Calculate the derivative of xeval
            double d_xeval_logpik = my_left + my_right;
            PRINT_DEBUG("d_xeval_logpik: " << d_xeval_logpik << endl, 2);
            // Now multiply this value for every k'
            for(int kprime = 0; kprime < K; kprime++) {
                d_xeval_dsikprime[kprime] += 
                    d_xeval_logpik * ((kprime == k?1:0)-p_i_k[i][kprime]);
            }
        }
        // Calculate the actual gradient
        for(int kprime = 0; kprime < K; kprime++) {
            double diff = d_xeval_dsikprime[kprime];
            double diffgamma = diff*gamma;
            BOOST_FOREACH(SparseMap::value_type val, all_feats_[i][kprime]) {
                g[val.first] += val.second * diffgamma;
                d_xeval_dgamma += val.second * x[val.first] * diff;
                // d_xeval_dw += nbest[kprime].first * (d_xeval_dsikprime[kprime] * gamma);
                // d_xeval_dgamma += weights * nbest[kprime].first * d_xeval_dsikprime[kprime];
            }
        }
    }
    if(auto_scale_ && d_xeval_dgamma != 0.0 && dense_scale_id_ != -1)
        g[dense_scale_id_] = d_xeval_dgamma;
}

void TuneXeval::Init() {
    // If we are not yet initialized
    if(sparse2dense_.size() == 0) {
        set<WordId> potential;
        BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_)
            examp->CountWeights(potential);
        potential.insert(scale_id_);
        BOOST_FOREACH(WordId val, potential) {
            sparse2dense_[val] = dense2sparse_.size();
            dense2sparse_.push_back(val);
        }
        // Build the stats
        all_stats_.clear(); all_stats_.resize(examps_.size());
        all_feats_.clear(); all_feats_.resize(examps_.size());
        for(int i = 0; i < (int)examps_.size(); i++) {
            const vector<ExamplePair> & nbest = examps_[i]->CalculateNbest(Weights());
            int K = nbest.size();
            all_stats_[i].resize(K);
            all_feats_[i].resize(K);
            for(int k = 0; k < K; k++) {
                all_stats_[i][k] = nbest[k].second;
                BOOST_FOREACH(SparsePair val, nbest[k].first.GetImpl())
                    all_feats_[i][k].push_back(make_pair(sparse2dense_[val.first], val.second));
            }
        }
        dense_scale_id_ = sparse2dense_[scale_id_];
    }
}

double TuneXeval::operator()(size_t n, const double * x, double * g) const {
    return CalcGradient(n, x, g);
}

double TuneXeval::CalcGradient(const SparseMap & kv, SparseMap & d_xeval_dw) const {
    size_t n = dense2sparse_.size();
    double* x = new double[n]; memset(x, 0, sizeof(double)*n);
    BOOST_FOREACH(const SparseMap::value_type val, kv) {
        SparseIntMap::const_iterator it = sparse2dense_.find(val.first);
        x[it->second] = val.second;
    }
    double* g = new double[n]; memset(g, 0, sizeof(double)*n);
    double ret = CalcGradient(n, x, g);
    for(size_t i = 0; i < n; i++)
        if(g[i] != 0.0)
            d_xeval_dw[dense2sparse_[i]] = g[i];
    delete[] x;
    delete[] g;
    return ret;
}

double TuneXeval::CalcGradient(size_t n, const double * x, double * g) const {

    iter_++;

    // Get the scaling factor
    double gamma = (dense_scale_id_ == -1 ? 1.0 : x[dense_scale_id_]);
    if(gamma == 0.0) gamma = 1.0;

    // Allocate some space for statistics
    EvalStatsPtr first_stats;
    int i;
    for(i = 0; i < (int)all_stats_.size(); i++) {
        if(all_stats_[i].size() > 0) {
            first_stats = all_stats_[i][0];
            break;
        }
    }
    if(first_stats.get() == NULL) return 0;
    int N = all_stats_.size();
    vector<vector<double> > p_i_k(N);
    EvalStatsPtr stats;
    // Create the expected stats, make sure it is of the right type by cloning
    // the first stats of the first n-best list
    stats = first_stats->Times(0);

    // ***** Calculate the expectation of the statistics *****
    for(int i = 0; i < N; i++) {

        // Get the probabilities of the n-best list
        int K = all_stats_[i].size();
        p_i_k[i].resize(K, 0.0);
        for(int k = 0; k < K; k++) {
            BOOST_FOREACH(SparseMap::value_type val, all_feats_[i][k])
                p_i_k[i][k] += x[val.first] * val.second;
            p_i_k[i][k] *= gamma;
        }
        p_i_k[i] = Softmax(p_i_k[i]);

        // Add to the expectation of the statistics
        for(int k = 0; k < K; k++) {
            stats->PlusEqualsTimes(*all_stats_[i][k], p_i_k[i][k]);
            PRINT_DEBUG("Iter " << iter_ << " "<<i<<" " << k << ": p=" << p_i_k[i][k] << ", stats=" << stats->ConvertToString() << endl, 3);
        }
    }
    
    if(first_stats->GetIdString() == "BLEU") {
        CalcBleuGradient(p_i_k, stats, n, x, g);
    } else if(first_stats->GetIdString() == "AVG" || first_stats->GetIdString() == "RIBES" || first_stats->GetIdString() == "TER") {
        CalcAvgGradient(p_i_k, stats, n, x, g);
    } else {
        THROW_ERROR("Cannot optimize expectation of "<<first_stats->GetIdString()<<" yet");
    }

    double score = stats->ConvertToScore();

    // Perform L2 regularization if necessary
    if(l2_coeff_ != 0.0) {
        double dgamma = 0;
        double gamma2 = gamma*gamma;
        for(size_t i = 0; i < n; i++) {
            if((int)i != dense_scale_id_) {
                double v2 = x[i]*x[i];
                score -= l2_coeff_ * v2 * gamma2;
                g[i] -= 2 * l2_coeff_ * x[i] * gamma2;
                dgamma -= 2 * l2_coeff_ * v2 * gamma;
            }
        }
        if(auto_scale_ && dgamma != 0.0 && dense_scale_id_ != -1)
            g[dense_scale_id_] += dgamma;
    }

    // Perform entropy regularization if necessary
    if(ent_coeff_ != 0.0) {
        double dgamma = 0;
        double log2 = log(2.0);
        for(int i = 0; i < N; i++) {
            int K = all_feats_[i].size();
            vector<double> ps(K, 0.0);
            for(int k = 0; k < K; k++) {
                double my_log2 = max(LogZero(p_i_k[i][k])/log2,-DBL_MAX);
                double val = (my_log2 + 1) * p_i_k[i][k];
                score += my_log2 * p_i_k[i][k] * ent_coeff_;
                for(int kprime = 0; kprime < K; kprime++)
                    ps[kprime] += val * ((kprime == k ? 1.0 : 0.0) - p_i_k[i][kprime]);
            }
            for(int kprime = 0; kprime < K; kprime++) {
                BOOST_FOREACH(const SparseMap::value_type & val, all_feats_[i][kprime]) {
                    g[val.first] -= ps[kprime] * val.second * gamma * ent_coeff_;
                    dgamma -= ps[kprime] * x[val.first] * val.second;
                }
            }
        }
        if(auto_scale_ && dgamma != 0.0 && dense_scale_id_ != -1)
            g[dense_scale_id_] += dgamma * ent_coeff_;
    }

    // If there is a multiplier to the gradient (i.e. -1)
    if(mult_ != 1.0) {
        for(size_t i = 0; i < n; i++)
            g[i] *= mult_;
    }

    PRINT_DEBUG("Iter " << iter_ << " Xeval: " << score << " @ " << stats->ConvertToString() << endl, 1);

    return score * mult_;
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
            last_score = CalcGradient(kv, d_xeval_dw);

            // Perform stochastic gradient descent
            kv += d_xeval_dw * step_size_;

            PRINT_DEBUG("Iter " << iter+1 << " KV: " << Dict::PrintSparseMap(kv) << endl, 2);
        }
    // Optimize using LBFGS. Iterations are done within the library
    } else if (optimizer_ == "lbfgs") {
        mult_ = -1;
        // Initialize the weights if we are using this setting
        vector<double> weights(dense2sparse_.size(),0.0);
        if(use_init_)
            for(int i = 0; i < (int)dense2sparse_.size(); i++)
                weights[i] = kv[dense2sparse_[i]];
        if(weights[sparse2dense_[scale_id_]] == 0.0)
            weights[sparse2dense_[scale_id_]] = 1.0;
        // Optimize and save
        liblbfgs::LBFGS<TuneXeval> lbfgs(*this, iters_, l1_coeff_, 1);
        last_score = lbfgs(weights.size(), &(*weights.begin()));
        kv = SparseMap();
        for(int i = 0; i < (int)dense2sparse_.size(); i++)
            if(weights[i])
                kv[dense2sparse_[i]] = weights[i];
    }

    return last_score * mult_;
}
