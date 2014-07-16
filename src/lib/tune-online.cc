#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tuning-example.h>
#include <travatar/tune-online.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/weights.h>
#include <travatar/weights-perceptron.h>
#include <travatar/weights-adagrad.h>
#include <travatar/weights-average-perceptron.h>
#include <travatar/weights-online-pro.h>
#include <travatar/eval-measure.h>
#include <travatar/sparse-map.h>
#include <travatar/output-collector.h>

// *** Update weights using online learning

using namespace std;
using namespace boost;
using namespace travatar;

// Tune new weights using online learning
double TuneOnline::RunTuning(SparseMap & kv) {
    PRINT_DEBUG("Starting Online Learning Run: " << Dict::PrintSparseMap(kv) << endl, 2);

    // Create a weight adjuster
    shared_ptr<Weights> weights;
    if(update_ == "perceptron") {
        WeightsPerceptron * ptr = new WeightsPerceptron;
        ptr->SetLearningRate(rate_);
        ptr->SetMarginScale(margin_scale_);
        weights = shared_ptr<Weights>(ptr);
    } else if(update_ == "avgperceptron") {
        WeightsAveragePerceptron * ptr = new WeightsAveragePerceptron;
        ptr->SetLearningRate(rate_);
        ptr->SetMarginScale(margin_scale_);
        weights = shared_ptr<Weights>(ptr);
    } else if(update_ == "adagrad") {
        WeightsAdagrad * ptr = new WeightsAdagrad;
        ptr->SetLearningRate(rate_);
        ptr->SetMarginScale(margin_scale_);
        weights = shared_ptr<Weights>(ptr);
    } else {
        THROW_ERROR("Unknown update type: " << update_);
    }

    if(algorithm_ == "pro") {
        shared_ptr<Weights> old_weights = weights;
        WeightsOnlinePro * wop = new WeightsOnlinePro(weights);
        wop->SetAlphaScale(examps_.size());
        weights.reset(wop);
    } else if(algorithm_ != "pairwise") {
        THROW_ERROR("Unknown update algorithm for online learning");
    }

    // To do in random order
    vector<int> order(examps_.size());
    for(int i = 0; i < (int)order.size(); i++) order[i] = i;

    // First, get the stats
    vector<EvalStatsPtr> all_stats;
    EvalStatsPtr total_stats;
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_) {
        // Get the best hypothesis according to model score
        const ExamplePair & expair = examp->CalculateModelHypothesis(*weights);
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
            vector<ExamplePair> nbest = examp.CalculateNbest(*weights);
            
            // Calculate the scores
            vector<pair<double, double> > scores(nbest.size());
            vector<SparseVector*> feats(nbest.size());
            PRINT_DEBUG("CURRENT: " << Dict::PrintSparseMap(weights->GetCurrent()) << endl, 3);
            for(int i = 0; i < (int)nbest.size(); i++) {
                total_stats->PlusEquals(*nbest[i].second);
                scores[i].first  = (*weights) * nbest[i].first;
                scores[i].second = total_stats->ConvertToScore();
                PRINT_DEBUG("SCORE " << idx << "/" << i << ":\t" << scores[i].second << endl, 4);
                feats[i] = &nbest[i].first;
                total_stats->PlusEquals(*nbest[i].second->Times(-1));
            }

            // Actually adjust the weights
            weights->Adjust(scores, feats);
            PRINT_DEBUG("AFTER:   " << Dict::PrintSparseMap(weights->GetCurrent()) << endl, 3);

            // Re-add the stats for the current example
            const ExamplePair & expair = examp.CalculateModelHypothesis(*weights);
            // cerr << idx << endl;
            if(&expair == NULL || expair.second.get() == NULL) THROW_ERROR("Null example pair @ " << idx << endl);
            all_stats[idx] = expair.second;
            total_stats->PlusEquals(*expair.second);
            
        }
        PRINT_DEBUG("Iter " << iter+1 << ": " << total_stats->ConvertToString() << endl, 1);
    }

    // Finally, update the stats with the actual weights
    kv = weights->GetFinal();
    total_stats.reset((EvalStats*)NULL);
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_) {
        const ExamplePair & expair = examp->CalculateModelHypothesis(*weights);
        if(total_stats.get()==NULL) total_stats=expair.second->Clone();
        else total_stats->PlusEquals(*expair.second);
    }

    return total_stats->ConvertToScore();
}
