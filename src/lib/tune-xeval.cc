#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tune-eval.h>
#include <travatar/util.h>
#include <travatar/dict.h>

using namespace std;
using namespace boost;
using namespace travatar;

// Tune new weights using the expected BLEU algorithm
double TuneXeval::RunTuning(SparseMap & kv) {

    // Sanity checks
    shared_ptr<Weights> weights(new Weights(kv));
    if(examps_.size() < 1)
        THROW_ERROR("Must have at least one example to perform tuning");
    EvalStatsPtr first_stats = examps_[0]->CalculateNbest(*weights)[0].second;
    if(first_stats->GetIdString() != "BLEU")
        THROW_ERROR("Expected eval tuning can only be used with BLEU at the moment");

    // Start
    PRINT_DEBUG("Starting Expected Eval Tuning Run: " << Dict::PrintFeatures(kv) << endl, 2);

    // Allocate some space for statistics
    int N = examps_.size();
    vector<vector<double> > p_i_k(N);
    vector<vector<EvalStatsPtr> > stats_i_k(N);
    vector<EvalStatsPtr> > stats_i(N); 

    // Do iterations of learning
    for(int iter = 0; iter < iters_; iter++) {

        // Create the expected stats, make sure it is of the right type by cloning
        // the first stats of the first n-best list
        EvalStatsPtr stats = first_stats->Times(0);

        // ***** Calculate the expectation of the statistics *****
        for(int i = 0; i < N; i++) {
            TuningExample * examp = examps_[i];

            // Get the n-best list
            const vector<ExamplePair> & nbest = examp.CalculateNbest(*weights);

            // Get the probabilities of the n-best list
            if(p_i_k[i].size()!=nbest.size()) p_i_k[i].resize(nbest.size());
            for(int k = 0; k < (int)nbest.size(); k++)
                p_i_k[i][k] = (*weights) * nbest[k].first;
            p_i_k[i] = Util::Softmax(p_i_k[i]);

            // Add to the expectation of the statistics
            if(stats_i_k[i].size() != nbest.size()) stats_i_k[i].resize(nbest.size());
            stats_i[i] = first_stats->Times(0);
            for(int k = 0; k < (int)nbest.size(); k++) {
                stats_i_k[i][k] = nbest[k].second->Times(p_i_k[i][k]);
                stats_i[i]->PlusEquals(*stats_i_k[i][k]);
            }
            stats->PlusEquals(*stats_i[i]);
        }
        PRINT_DEBUG("Iter " << iter+1 << " Xeval: " << stats->ConvertToString() << endl, 1);

        // ***** Calculate the gradient *****
        // Overall stats
        EvalStatsBleu * stats_bleu = (EvalStatsBleu*)stats.get();
        double P = stats_bleu->GetAvgLogPrecision();
        double eP = exp(P);
        double R = 1.0/stats_bleu->GetLengthRatio();
        double omR = 1-R;
        double eomR = exp(omR), e10komR = exp(10000*omR);
        double B = (eomR-1)/(e10komR+1) + 1;
        // This is used in the calculation of dB/log(p_{i,k})
        double d_B_logpik_Rpart = ((e10komR+1)*eomR - 10000*e10komR*(eomR-1)) /
                                  (e10komR+1)/(e10komR+1) * -R;
        // Gradient
        SparseMap d_xBLEU_dw;
        // Calculate the stats for each example
        for(int i = 0; i < N; i++) {
            int K = p_i_k[i].size();
            EvalStatsBleu * stats_i_bleu = (EvalStatsBleu*)stats_i[i].get();
            // The amount to multiply each member k' by
            vector<double> d_xBLEU_dsikprime(K,0);
            for(int k = 0; k < K; k++) {
                EvalStatsBleu * stats_i_k_bleu = (EvalStatsBleu*)stats_i_k[i][k].get();
                // Calculate the derivative of exp(P) with respect to log(p_{i,k})
                double d_expP_logpik = 0;
                for(int n = 0; n < stats_bleu->GetNgramOrder(); n++)
                    d_expP_logpik += stats_i_k_bleu->GetMatch(n)/stats_i_bleu->GetMatch(n) -
                                     stats_i_k_bleu->GetCount(n)/stats_i_bleu->GetCount(n);
                d_expP_logpik /= stats_bleu->GetNgramOrder();
                // Calculate the derivative of B with respect to log(p_{i,k})
                double d_B_logpik = d_B_logpik_Rpart * (
                                        stats_i_k_bleu->GetRefLen()/stats_i_bleu->GetRefLen() -
                                        stats_i_k_bleu->GetCount(1)/stats_i_bleu->GetCount(1));
                // Calculate the derivative of xBLEU
                double d_xBLEU_logpik = eP * ( d_B_logpik + d_expP_logpik * B );
                // Now multiply this value for every k'
                for(int kprime = 0; kprime < K; kprime++)
                    d_xBLEU_dsikprime[kprime] += 
                        d_XBLEU_logpik * ((kprime == k?1:0)-p_i_k[kprime]);
            }
            // Calculate the actual gradient
            const vector<ExamplePair> & nbest = examps_[i].CalculateNbest(*weights);
            for(int kprime = 0; kprime < K; kprime++)
                d_xBLEU_dw += (*nbest.first) * d_xBLEU_dsikprime[kprime]; 
        }
        
        // TODO: Change this to LBFGS
        // Perform stochastic gradient descent
        kv += d_xBLEU_dw * step_size_;
        weights.reset(new Weights(kv));

    }

    return stats->ConvertToScore();
}
