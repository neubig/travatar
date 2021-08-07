#include <travatar/gradient-xeval.h>
#include <travatar/global-debug.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/softmax.h>
#include <boost/foreach.hpp>
#include <cmath>

using namespace std;
using namespace boost;
using namespace travatar;

inline Real DivideZero(Real x, Real y) {
    return (y==0?0:x/y);
}
inline Real LogZero(Real x) {
    return (x<=0?-REAL_MAX:log(x));
}

GradientXeval::GradientXeval() : Gradient(), ent_coeff_(0.0) { }

void GradientXeval::CalcAvgGradient(
            const vector<vector<Real> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const Real * x, Real * g) const {
    EvalStatsAverage * stats_avg = (EvalStatsAverage*)stats.get();
    int N = p_i_k.size();
    // The scaling constant and its
    Real gamma = (dense_scale_id_ == -1 ? 1.0 : x[dense_scale_id_]);
    if(gamma == 0.0) gamma = 1.0;
    Real d_xeval_dgamma = 0;

    // Calculate the stats for each example
    for(int i = 0; i < N; i++) {
        int K = p_i_k[i].size();
        // The amount to multiply each member k' by
        vector<Real> d_xeval_dsikprime(K,0);
        for(int k = 0; k < K; k++) {
            EvalStatsAverage * stats_i_k_avg = (EvalStatsAverage*)all_stats_[i][k].get();
            PRINT_DEBUG("Stats i=" << i << ", k=" << k << ": " << stats_i_k_avg->ConvertToString() << endl, 4);
            // Calculate the derivative of xeval
            Real d_xeval_logpik = p_i_k[i][k]*stats_i_k_avg->GetVal()/stats_avg->GetDenom();
            // Now multiply this value for every k'
            for(int kprime = 0; kprime < K; kprime++)
                d_xeval_dsikprime[kprime] += 
                    d_xeval_logpik * ((kprime == k?1:0)-p_i_k[i][kprime]);
        }
        // Calculate the actual gradient
        for(int kprime = 0; kprime < K; kprime++) {
            Real diff = d_xeval_dsikprime[kprime];
            Real diffgamma = diff*gamma;
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

void GradientXeval::CalcBleuGradient(
            const vector<vector<Real> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const Real * x, Real * g) const {
    // Overall stats
    EvalStatsBleu * stats_bleu = (EvalStatsBleu*)stats.get();
    int N = p_i_k.size();
    Real P = stats_bleu->GetAvgLogPrecision();
    Real eP = exp(P);
    Real R = 1.0/stats_bleu->GetLengthRatio();
    Real mR = 1-R;
    Real emR = exp(mR), e10kmR = std::min(REAL_MAX,exp(10000*mR));
    Real B = (emR-1)/(e10kmR+1) + 1;
    Real Bprime = (emR-(emR-1)*10000*(e10kmR/(1+e10kmR)))/(1+e10kmR);
    // This is used in the calculation of dB/log(p_{i,k})
    PRINT_DEBUG("P=" << P << ", eP=" << eP << ", B=" << B << ", Bprime=" << Bprime << ", R=" << R << ", e10kmR="<<e10kmR<<", emR=" << emR << endl, 3);
    // The left and right constants
    Real left = eP * Bprime * -R;
    Real right = eP * B / stats_bleu->GetNgramOrder();
    // The scaling constant and its
    Real gamma = (dense_scale_id_ == -1 ? 1.0 : x[dense_scale_id_]);
    if(gamma == 0.0) gamma = 1.0;
    Real d_xeval_dgamma = 0;

    // Calculate the stats for each example
    for(int i = 0; i < N; i++) {
        int K = p_i_k[i].size();
        // The amount to multiply each member k' by
        vector<Real> d_xeval_dsikprime(K,0);
        for(int k = 0; k < K; k++) {
            EvalStatsBleu * stats_i_k_bleu = (EvalStatsBleu*)all_stats_[i][k].get();
            PRINT_DEBUG("s_i_k_bleu: " << stats_i_k_bleu->ConvertToString() << endl, 3);
            Real pik = p_i_k[i][k];
            // Calculate the derivative of exp(P) with respect to log(p_{i,k})
            Real my_right = 0;
            for(int n = 0; n < stats_bleu->GetNgramOrder(); n++)
                my_right += DivideZero(stats_i_k_bleu->GetMatch(n)*pik,stats_bleu->GetMatch(n)) -
                            DivideZero(stats_i_k_bleu->GetSysCount(n)*pik,stats_bleu->GetSysCount(n));
            my_right *= right;
            PRINT_DEBUG("my_right: " << my_right << endl, 3);
            // Calculate the derivative of B with respect to log(p_{i,k})
            Real my_left = left * (
                             DivideZero(stats_i_k_bleu->GetRefCount(0)*pik,stats_bleu->GetRefCount(0)) -
                             DivideZero(stats_i_k_bleu->GetSysCount(0)*pik,stats_bleu->GetSysCount(0)));
            PRINT_DEBUG("my_left: " << my_left << endl, 3);
            // Calculate the derivative of xeval
            Real d_xeval_logpik = my_left + my_right;
            PRINT_DEBUG("d_xeval_logpik: " << d_xeval_logpik << endl, 3);
            // Now multiply this value for every k'
            for(int kprime = 0; kprime < K; kprime++) {
                d_xeval_dsikprime[kprime] += 
                    d_xeval_logpik * ((kprime == k?1:0)-p_i_k[i][kprime]);
            }
        }
        // Calculate the actual gradient
        for(int kprime = 0; kprime < K; kprime++) {
            Real diff = d_xeval_dsikprime[kprime];
            Real diffgamma = diff*gamma;
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

Real GradientXeval::CalcGradient(size_t n, const Real * x, Real * g) const {

    // Get the scaling factor
    Real gamma = (dense_scale_id_ == -1 ? 1.0 : x[dense_scale_id_]);
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
    vector<vector<Real> > p_i_k(N);
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
            PRINT_DEBUG("i="<<i<<", k=" << k << ": p=" << p_i_k[i][k] << ", stats=" << stats->ConvertToString() << endl, 4);
        }
    }
    
    if(first_stats->GetIdString() == "BLEU") {
        CalcBleuGradient(p_i_k, stats, n, x, g);
    } else if(first_stats->GetIdString() == "AVG" || first_stats->GetIdString() == "RIBES" || first_stats->GetIdString() == "TER" || first_stats->GetIdString() == "ZEROONE") {
        CalcAvgGradient(p_i_k, stats, n, x, g);
    } else {
        THROW_ERROR("Cannot optimize expectation of "<<first_stats->GetIdString()<<" yet");
    }

    Real score = stats->ConvertToScore();

    // Perform L2 regularization if necessary
    if(l2_coeff_ != 0.0) {
        Real dgamma = 0;
        Real gamma2 = gamma*gamma;
        for(size_t i = 0; i < n; i++) {
            if((int)i != dense_scale_id_) {
                Real v2 = x[i]*x[i];
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
        Real dgamma = 0;
        Real log2 = log(2.0);
        for(int i = 0; i < N; i++) {
            int K = all_feats_[i].size();
            vector<Real> ps(K, 0.0);
            for(int k = 0; k < K; k++) {
                Real my_log2 = max(LogZero(p_i_k[i][k])/log2,-REAL_MAX);
                Real val = (my_log2 + 1) * p_i_k[i][k];
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

    PRINT_DEBUG("Xeval: " << score << " @ " << stats->ConvertToString() << endl, 2);

    return score * mult_;
}
