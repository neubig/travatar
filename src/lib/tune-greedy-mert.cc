#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tuning-example.h>
#include <travatar/tune-greedy-mert.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/sparse-map.h>

using namespace std;
using namespace boost;
using namespace travatar;

#define MARGIN 1

// TODO: Make it possible to calculate with only some examples
// Return the optimal value and the score achieved at that value
pair<double,double> TuneGreedyMert::LineSearch(
                const vector<shared_ptr<TuningExample> > & examps, 
                const SparseMap & weights,
                const SparseMap & gradient,
                pair<double,double> range) {
    double base_score = 0;
    map<double,double> boundaries;
    typedef pair<double,double> DoublePair;
    // Create the search plane
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps) {
        // Calculate the convex hull
        ConvexHull convex_hull = examp->CalculateConvexHull(weights, gradient);
        PRINT_DEBUG("Convex hull size == " << convex_hull.size() << endl, 2);
        if(convex_hull.size() == 0) continue;
        // Update the base values
        base_score += convex_hull[0].second;
        // Add all the changed values
        for(int i = 1; i < (int)convex_hull.size(); i++) {
            double diff = convex_hull[i].second-convex_hull[i-1].second;
            if(diff != 0)
                boundaries[convex_hull[i].first.first] += diff;
        }
    }
    boundaries[DBL_MAX] = -DBL_MAX;
    // if(boundaries.size() == 0) return make_pair(-DBL_MAX, -DBL_MAX);
    // Find the place with the best score on the plane
    ScoredSpan best_span(Span(-DBL_MAX, -DBL_MAX), 0);
    double last_bound = -DBL_MAX, curr_score = base_score;
    zero_score_ = DBL_MAX;
    BOOST_FOREACH(const DoublePair & boundary, boundaries) {
        // Find the score at zero. If there is a boundary directly at zero, break ties
        // to the less optimistic side (or gain to the optimistic side)
        if(last_bound <= 0 && boundary.first >= 0)
            zero_score_ = curr_score;
        // Update the span if it exceeds the previous best and is in the acceptable gradient range
        if(curr_score > best_span.second && (last_bound < range.second && boundary.first > range.first))
            best_span = ScoredSpan(Span(last_bound, boundary.first), curr_score);
        // cerr << "bef: " << boundary << " curr_score=" << curr_score << endl;
        curr_score += boundary.second;
        // cerr << "aft: " << boundary << " curr_score=" << curr_score << endl;
        last_bound = boundary.first;
    }
    // Given the best span, find the middle
    double middle;
    if(best_span.first.first < 0 && best_span.first.second > 0)
        middle = 0;
    else if (best_span.first.first == -DBL_MAX)
        middle = best_span.first.second - MARGIN;
    else if (best_span.first.second == DBL_MAX)
        middle = best_span.first.first + MARGIN;
    else
        middle = (best_span.first.first+best_span.first.second)/2;
    middle = max(range.first, min(middle, range.second));
    PRINT_DEBUG("0 --> " << zero_score_ << ", " << middle << " --> " << best_span.second << endl, 2);
    return make_pair(middle, best_span.second-zero_score_);
}

// Current value can be found here
// range(-2,2)
// original(-1)
// change(-1,3)
// gradient -2
// +0.5, -1.5
pair<double,double> TuneGreedyMert::FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                pair<double,double> range) {
    pair<double,double> ret(-DBL_MAX, DBL_MAX);
    BOOST_FOREACH(const SparseMap::value_type & grad, gradient) {
        if(grad.second == 0) continue;
        SparseMap::const_iterator it = weights.find(grad.first);
        double w = (it != weights.end()) ? it->second : 0.0;
        double l = (range.first-w)/grad.second;
        double r = (range.second-w)/grad.second;
        if(l > r) { double temp = l; l = r; r = temp; }
        ret.first = max(l, ret.first);
        ret.second = min(r, ret.second);
    }
    return ret; 
}

// Find the best value to tune and tune it
double TuneGreedyMert::TuneOnce(
           const vector<shared_ptr<TuningExample> > & examps,
           SparseMap & weights) {
    PRINT_DEBUG("Calculating potential gains..." << endl, 1);
    SparseMap potential;
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps)
        potential += examp->CalculatePotentialGain(weights);
    // Order the weights in descending order
    typedef pair<double,int> DIPair;
    vector<DIPair> vals;
    BOOST_FOREACH(const SparseMap::value_type val, potential)
        if(val.second >= gain_threshold_)
            vals.push_back(make_pair(val.second, val.first));
    sort(vals.begin(), vals.end());
    // Perform line search for each weight
    pair<double,double> best_result(0,0);
    SparseMap best_gradient;
    string best_str = "NA";
    BOOST_REVERSE_FOREACH(const DIPair & val, vals) {
        if(val.first < best_result.second) {
            PRINT_DEBUG(Dict::WSym(val.second) << "=" << val.first << " (max: " << best_str << "=" << best_result.second << "): BREAK!" << endl, 0);
            break;
        }
        SparseMap gradient;
        gradient[val.second] = 1;
        RangeMap::const_iterator it = ranges_.find(val.second);
        pair<double,double> range = (it == ranges_.end() ? ranges_[-1] : it->second);
        pair<double,double> gradient_range = FindGradientRange(weights, gradient, range);
        pair<double,double> search_result = LineSearch(examps, weights, gradient, gradient_range);
        if(search_result.second > best_result.second) {
            best_result = search_result;
            best_gradient = gradient;
            best_str = Dict::WSym(val.second);
        }
        PRINT_DEBUG("gain?("<<Dict::WSym(val.second)<<")=" << val.first << " --> gain@" << search_result.first <<"="<< search_result.second << ", score="<<zero_score_+search_result.second<<" (max: " << best_str << "=" <<  best_result.second << ")" << endl, 1);
    }
    // Update with the best value
    if(best_result.second > gain_threshold_) {
        PRINT_DEBUG("Updating: " << Dict::PrintFeatures(best_gradient) << " * " << best_result.first << endl, 0);
        weights += best_gradient * best_result.first;
    }
    PRINT_DEBUG("Features: " << Dict::PrintFeatures(weights) << endl, 0);
    return best_result.second;
}

// Tune new weights using greedy mert until the threshold is exceeded
void TuneGreedyMert::Tune(
     const vector<shared_ptr<TuningExample> > & examps,
     SparseMap & weights) {
    while(TuneOnce(examps, weights) > gain_threshold_);
}
