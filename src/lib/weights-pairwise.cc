#include <travatar/weights-pairwise.h>

using namespace std;
using namespace boost;
using namespace travatar;

// Adjust the weights according to the n-best list
void WeightsPairwise::Adjust(const EvalMeasure & eval,
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
    Update(nbest[oracle]->CalcFeatures(), nbest[oracle]->GetScore(), oracle_loss,
           nbest[0]->CalcFeatures(), nbest[0]->GetScore(), eval.MeasureLoss(refs, nbest[0]->GetWords()));
}
