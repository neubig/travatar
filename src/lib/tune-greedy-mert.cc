#include <cfloat>
#include <boost/foreach.hpp>
#include <travatar/tune-greedy-mert.h>
#include <travatar/util.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

// Calculate the potential gain for a single example given the current
// weights
SparseMap TuneGreedyMert::CalculatePotentialGain(
            const std::vector<ExamplePair> & examps,
            const SparseMap & weights) {
    // Find the hypothesis to be chosen with the current weights
    int hyp = -1;
    double hyp_score = -DBL_MAX;
    for(int i = 0; i < (int)examps.size(); i++) {
        double my_score = examps[i].first * weights;
        if(my_score > hyp_score) {
            hyp = i;
            hyp_score = my_score;
        }
    }
    // Find all features that have the potential to cause a gain
    SparseMap ret;
    BOOST_FOREACH(const ExamplePair & examp, examps) {
        double gain = examp.second - examps[hyp].second;
        if(gain <= 0) continue; // Skip examples with no or negative gain
        BOOST_FOREACH(const SparseMap::value_type val, examp.first - examps[hyp].first)
            if(val.second != 0) // Skip examples with same value as current ans
                ret[val.first] = max(ret[val.first], gain);
    }
    return ret;
}

// Tune new weights using greedy mert
SparseMap TuneGreedyMert::Tune(
           const std::vector<std::vector<ExamplePair> > & examps,
           const SparseMap & init) {
    SparseMap potential;
    BOOST_FOREACH(const std::vector<ExamplePair> & examp, examps)
        potential += CalculatePotentialGain(examp, init);
    BOOST_FOREACH(const SparseMap::value_type val, potential)
        cout << Dict::WSym(val.first) << "=" << val.second << endl;
    THROW_ERROR("Not implemented yet");
    return init;
}
