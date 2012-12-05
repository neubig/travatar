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
    double oracle_loss = DBL_MAX;
    for(int i = 0; i < (int)nbest.size(); i++) {
        const shared_ptr<HyperPath> & path = nbest[i];
        double my_loss = eval.MeasureLoss(refs, path->GetWords());
        if(my_loss < oracle_loss) {
            oracle = i;
            oracle_loss = my_loss;
        }
    }
    // Update given the oracle
    double oracle_score = nbest[oracle]->GetScore();
    double sys_score = nbest[0]->GetScore();
    double sys_loss = eval.MeasureLoss(refs, nbest[0]->GetWords());
    Update(nbest[oracle]->CalcFeatures(), oracle_score, oracle_loss,
           nbest[0]->CalcFeatures(), sys_score, sys_loss);
    PRINT_DEBUG("WeightsPairwise::Adjust: os=" << oracle_score << ", ol=" << oracle_loss
                                       << ", ss=" << sys_score << ", sl=" << sys_loss << endl
                                       /* << Dict::PrintFeatures(current_) << endl */
                                       , 1);
    PRINT_DEBUG("Oracle["<<oracle<<"]\t"<<Dict::PrintFeatures(nbest[oracle]->CalcFeatures())<<endl
                <<Dict::PrintWords(nbest[oracle]->GetWords())<<endl
                <<"System[ 0 ]\t"    <<Dict::PrintFeatures(nbest[0]->CalcFeatures())<<endl
                <<Dict::PrintWords(nbest[0]->GetWords())<<endl
                /* <<"Final\t"          <<Dict::PrintFeatures(GetFinal())<<endl */
                ,2);
}
