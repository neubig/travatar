#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tuning-example.h>
#include <travatar/tune-online.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/eval-measure.h>
#include <travatar/sparse-map.h>
#include <travatar/output-collector.h>

// *** Update weights using online learning

using namespace std;
using namespace boost;
using namespace travatar;

// Tune new weights using online learning
double TuneOnline::RunTuning(SparseMap & weights) {
    double best_score = 0;
    PRINT_DEBUG("Starting Online Leraning Run: " << Dict::PrintFeatures(weights) << endl, 2);

    // To do in random order
    vector<int> order(examps_.size());
    for(int i = 0; i < (int)order.size(); i++) order[i] = i;

    // First, get the stats. Because this takes some time, update every
    // major iteration.
    vector<EvalStatsPtr> all_stats;
    EvalStatsPtr total_stats;
    BOOST_FOREACH(const TuningExample & examp, examps_) {
        const ExamplePair & expair = examp.GetModelHypothesis();
        all_stats.push_back(exppair.second);
        if(total_stats.get()==NULL) total_stats=exppair.second.Clone();
        else total_stats.PlusEquals(exppair.second);
    }

    // TODO: For 20 iterations
    for(int iter = 0; iter < 20; iter++) {

        // Shuffle the indexes
        if(shuffle_) random_shuffle(order.begin(), order.end());

        // Perform learning
        BOOST_FOREACH(int idx, order) {
            const TuningExample & examp = examps_[idx];
            // Remove the stats for the current example
            total_stats.PlusEquals(all_stats.TimesEquals(-1));

            // Get the n-best list and update
            vector<ExamplePair> nbest = examp.CalcNbest();
            
            // Calculate the scores
            vector<double> scores(nbest.size());
            for(int i = 0; i < nbest.size(); i++) {
                total_stats->PlusEquals(*nbest[i]);
                scores[i] = total_stats->ConvertToScore();
                total_stats->PlusEquals(nbest[i]->TimesEquals(-1));
            }
            
            // Perform the update
            UpdatePerceptron(nbest, total_stats, weights);

            // Re-add the stats for the current example
            const ExamplePair & expair = examp.CalcModelHypothesis(weights);
            all_stats[idx] = exppair.second;
            total_stats->PlusEquals(exppair.second);
            
        }
        PRINT_DEBUG("Score after learning iter " << iter+1 << ": " << total_stats->ConvertToString());
    }

    return total_stats->ConvertToScore();
}
