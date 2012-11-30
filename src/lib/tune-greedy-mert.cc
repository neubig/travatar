#include <cfloat>
#include <boost/foreach.hpp>
#include <travatar/tune-greedy-mert.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <map>

using namespace std;
using namespace travatar;

#define MARGIN 1

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

inline double FindIntersection(double s1, double v1, double s2, double v2) {
    return (s1==s2 ? DBL_MAX : (v1-v2)/(s2-s1));

}

// Get the convex hull, which consists of scored spans in order of the span location
std::vector<TuneGreedyMert::ScoredSpan> TuneGreedyMert::CalculateConvexHull(
                        const std::vector<ExamplePair> & examps, 
                        const SparseMap & weights,
                        const SparseMap & gradient) {
    // First, get all the lines, these are tuples of
    // the slope of the line, the value at zero, and the score
    // These happen to be the same shape as ScoredSpans so we abuse notation
    vector<ScoredSpan> lines;
    BOOST_FOREACH(const ExamplePair & examp, examps) {
        double slope = examp.first * gradient;
        double val = examp.first * weights;
        lines.push_back(make_pair(make_pair(slope, val), examp.second));
    }
    // Sort in order of ascending slope
    sort(lines.begin(), lines.end());
    // Start at position zero
    // While we are not finished
    vector<ScoredSpan> hull;
    double prev_pos = -DBL_MAX;
    // In case of ties in slope, we want to start with the line with the highest value
    int i;
    for(i = 0; i+1 < (int)lines.size() && lines[i].first.first == lines[i+1].first.first; i++);
    // Cycle through the points
    while(i < (int)lines.size()) {
        // Find the line the crosses at the earliest point
        int best_j = lines.size();
        double best_pos = DBL_MAX;
        for(int j = i+1; j < (int)lines.size(); j++) {
            // Find the intersection of the two lines
            double my_pos =
                FindIntersection(
                    lines[i].first.first, lines[i].first.second,
                    lines[j].first.first, lines[j].first.second
                );
            // If this is the earliest intersection, record it
            // Note that we are over-writing previous tying intersections with
            // smaller slopes
            if(my_pos <= best_pos) {
                best_j = j;
                best_pos = my_pos;
            }
        }
        PRINT_DEBUG("Adding hull: " << prev_pos << ", " << best_pos << ", " << lines[i].second << endl, 2);
        hull.push_back(make_pair(make_pair(prev_pos, best_pos), lines[i].second));
        i = best_j;
        prev_pos = best_pos;
    }
    return hull;
}

// TODO: Make it possible to calculate with only some examples
// Return the optimal value and the score achieved at that value
pair<double,double> TuneGreedyMert::LineSearch(
                const std::vector<std::vector<ExamplePair> > & examps, 
                const SparseMap & weights,
                const SparseMap & gradient) {
    double base_score = 0;
    map<double,double> boundaries;
    typedef std::pair<double,double> DoublePair;
    typedef std::vector<ExamplePair> Instance;
    // Create the search plane
    BOOST_FOREACH(const Instance & examp, examps) {
        // Calculate the convex hull
        vector<ScoredSpan> convex_hull = CalculateConvexHull(examp, weights, gradient);
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
    double last_bound = -DBL_MAX, curr_score = base_score, zero_score = DBL_MAX;
    BOOST_FOREACH(const DoublePair & boundary, boundaries) {
        // Find the score at zero. If there is a boundary directly at zero, break ties
        // to the less optimistic side (or gain to the optimistic side)
        if(last_bound <= 0 && boundary.first >= 0)
            zero_score = min(curr_score, zero_score);
        // Update the span up until the last boundary
        if(curr_score > best_span.second) 
            best_span = ScoredSpan(Span(last_bound, boundary.first), curr_score);
        curr_score += boundary.second;
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
    PRINT_DEBUG("0 --> " << zero_score << ", " << middle << " --> " << best_span.second << endl, 0);
    return make_pair(middle, best_span.second-zero_score);
}

// Find the best value to tune and tune it
double TuneGreedyMert::TuneOnce(
           const std::vector<std::vector<ExamplePair> > & examps,
           SparseMap & weights) {
    PRINT_DEBUG("Calculating potential gains..." << endl, 1);
    SparseMap potential;
    BOOST_FOREACH(const std::vector<ExamplePair> & examp, examps)
        potential += CalculatePotentialGain(examp, weights);
    // Order the weights in descending order
    typedef pair<double,int> DIPair;
    vector<DIPair> vals;
    BOOST_FOREACH(const SparseMap::value_type val, potential)
        if(val.second >= gain_threshold_)
            vals.push_back(std::make_pair(val.second, val.first));
    sort(vals.begin(), vals.end());
    // Perform line search for each weight
    pair<double,double> best_result(0,0);
    SparseMap best_gradient;
    BOOST_REVERSE_FOREACH(const DIPair & val, vals) {
        if(val.first < best_result.second) {
            PRINT_DEBUG(Dict::WSym(val.second) << "=" << val.first << " (max: " << best_result.second << "): BREAK!" << endl, 1);
            break;
        }
        SparseMap gradient;
        gradient[val.second] = 1;
        pair<double,double> search_result = LineSearch(examps, weights, gradient);
        PRINT_DEBUG(Dict::WSym(val.second) << "=" << val.first << " --> " << search_result << " (max: " << best_result.second << ")" << endl, 1);
        if(search_result.second > best_result.second) {
            best_result = search_result;
            best_gradient = gradient;
        }
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
     const std::vector<std::vector<ExamplePair> > & examps,
     SparseMap & weights) {
    while(TuneOnce(examps, weights) > gain_threshold_);
}
