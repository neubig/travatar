#include <travatar/weights-online-pro.h>
#include <travatar/weights-pairwise.h>
#include <travatar/eval-measure.h>
#include <travatar/hyper-graph.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <boost/random/mersenne_twister.hpp>
#include <cstdlib>
#include <algorithm>
#include <ctime>

using namespace std;
using namespace boost;
using namespace travatar;

// Run the Pro sampler from
//  Tuning as Ranking
//  Mark Hopkins and Jonathan May
//  EMNLP 2012
// and tune the weights using online learning
void WeightsOnlinePro::Adjust(
        const std::vector<std::pair<double,double> > & scores,
        const std::vector<SparseVector*> & features) {
    boost::mt19937 gen(time(0));
    // Sampling algorithm 1
    vector<pair<double,pair<int,int> > > samples;
    for(int i = 0; i < num_samples_; i++) {
        int id1 = gen() % scores.size(),
            id2 = gen() % scores.size();
        // Make sure id1 is the oracle
        if(scores[id2].second > scores[id1].second)
            swap(id1, id2);
        // If the difference is big enough, and probability
        double diff = (scores[id1].second-scores[id2].second) * alpha_scale_;
        if( diff > diff_threshold_ && ((double)gen())/RAND_MAX < diff )
            samples.push_back(
                make_pair(scores[id2].second - scores[id1].second,
                    make_pair(id1,id2)));
    }
    // Sort in ascending order of margin
    sort(samples.begin(), samples.end());
    for(int i = 0; i < min(num_updates_, (int)samples.size()); i++) {
        int id1 = samples[i].second.first,
            id2 = samples[i].second.second;
        weights_.get()->Update(
                         *features[id1], scores[id1].first, 1,
                         *features[id2], scores[id2].first, 0);
    }

}
