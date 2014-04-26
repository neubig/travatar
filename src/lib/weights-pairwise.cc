#include <travatar/weights-pairwise.h>
#include <travatar/eval-measure.h>
#include <travatar/hyper-graph.h>
#include <travatar/util.h>
#include <travatar/dict.h>

using namespace std;
using namespace boost;
using namespace travatar;

// Adjust the weights according to the n-best list
void WeightsPairwise::Adjust(EvalMeasure & eval,
                             const std::vector<Sentence> & refs,
                             const NbestList & nbest) {
    // Find the oracle
    int oracle = -1;
    double oracle_eval = -DBL_MAX;
    for(int i = 0; i < (int)nbest.size(); i++) {
        const shared_ptr<HyperPath> & path = nbest[i];
        double my_eval = -DBL_MAX;
        BOOST_FOREACH(const Sentence & ref, refs)
            my_eval = max(my_eval, eval.CalculateStats(ref, path->GetTrgData()[factor_].words)->ConvertToScore());
        if(my_eval > oracle_eval) {
            oracle = i;
            oracle_eval = my_eval;
        }
    }
    // Update given the oracle
    double oracle_score = nbest[oracle]->GetScore();
    double sys_score = nbest[0]->GetScore();
    double sys_eval = -DBL_MAX;
    BOOST_FOREACH(const Sentence & ref, refs)
        sys_eval = max(sys_eval, eval.CalculateStats(ref, nbest[0]->GetTrgData()[factor_].words)->ConvertToScore());
    Update(nbest[oracle]->CalcFeatures(), oracle_score, oracle_eval,
           nbest[0]->CalcFeatures(), sys_score, sys_eval);
    PRINT_DEBUG("WeightsPairwise::Adjust: os=" << oracle_score << ", oe=" << oracle_eval
                                       << ", ss=" << sys_score << ", se=" << sys_eval << endl
                                       /* << Dict::PrintFeatures(current_) << endl */
                                       , 1);
    PRINT_DEBUG("Oracle["<<oracle<<"]\t"<<Dict::PrintFeatures(nbest[oracle]->CalcFeatures())<<endl
                <<Dict::PrintWords(nbest[oracle]->GetTrgData()[factor_].words)<<endl
                <<"System[ 0 ]\t"    <<Dict::PrintFeatures(nbest[0]->CalcFeatures())<<endl
                <<Dict::PrintWords(nbest[0]->GetTrgData()[factor_].words)<<endl
                /* <<"Final\t"          <<Dict::PrintFeatures(GetFinal())<<endl */
                ,2);
}
