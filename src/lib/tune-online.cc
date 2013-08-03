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
    PRINT_DEBUG("Starting Online Learning Run: " << Dict::PrintFeatures(weights) << endl, 2);

    // To do in random order
    vector<int> order(examps_.size());
    for(int i = 0; i < (int)order.size(); i++) order[i] = i;

    // First, get the stats
    vector<EvalStatsPtr> all_stats;
    EvalStatsPtr total_stats;
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_) {
        // Get the best hypothesis according to model score
        const ExamplePair & expair = examp->CalculateModelHypothesis(weights);
        all_stats.push_back(expair.second);
        if(total_stats.get()==NULL) total_stats=expair.second->Clone();
        else total_stats->PlusEquals(*expair.second);
    }

    // Do iterations of online learning
    for(int iter = 0; iter < iters_; iter++) {

        // Shuffle the indexes
        if(shuffle_) random_shuffle(order.begin(), order.end());

        // Perform learning
        BOOST_FOREACH(int idx, order) {
            TuningExample & examp = *examps_[idx];
            // Remove the stats for the current example
            total_stats->PlusEquals(*all_stats[idx]->Times(-1));

            // Get the n-best list and update
            vector<ExamplePair> nbest = examp.CalculateNbest(weights);
            
            // Calculate the scores
            vector<pair<double, double> > scores(nbest.size());
            for(int i = 0; i < (int)nbest.size(); i++) {
                total_stats->PlusEquals(*nbest[i].second);
                scores[i].first  = nbest[i].first * weights;
                scores[i].second = total_stats->ConvertToScore();
                total_stats->PlusEquals(*nbest[i].second->Times(-1));
            }
            
            // TODO: Integrate this with Weights
            int oracle_id = 0, model_id = 0;
            for(int i = 1; i < (int)nbest.size(); i++) {
                if(scores[i].first > scores[model_id].first)
                    model_id = i;
                if(scores[i].second > scores[oracle_id].second)
                    oracle_id = i;
            }
            if(model_id != oracle_id) {
                weights += nbest[oracle_id].first;
                weights -= nbest[model_id].first;
            }

            // Re-add the stats for the current example
            const ExamplePair & expair = examp.CalculateModelHypothesis(weights);
            all_stats[idx] = expair.second;
            total_stats->PlusEquals(*expair.second);
            
        }
        PRINT_DEBUG("Approx score after learning iter " << iter+1 << ": " << total_stats->ConvertToString() << endl, 1);
    }

    // Finally, update the stats with the actual weights
    total_stats.reset((EvalStats*)NULL);
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_) {
        const ExamplePair & expair = examp->CalculateModelHypothesis(weights);
        if(total_stats.get()==NULL) total_stats=expair.second->Clone();
        else total_stats->PlusEquals(*expair.second);
    }

    return total_stats->ConvertToScore();
}
