#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tuning-example.h>
#include <travatar/tune-mert.h>
#include <travatar/global-debug.h>
#include <travatar/io-util.h>
#include <travatar/dict.h>
#include <travatar/eval-measure.h>
#include <travatar/sparse-map.h>
#include <travatar/output-collector.h>

using namespace std;
using namespace boost;
using namespace travatar;

#define MARGIN 1

LineSearchResult TuneMert::LineSearch(
                const SparseMap & weights,
                const SparseMap & gradient,
                vector<shared_ptr<TuningExample> > & examps,
                pair<double,double> range) {
    EvalStatsPtr base_stats;
    map<double,EvalStatsPtr> boundaries;
    typedef pair<double,EvalStatsPtr> DoublePair;
    // Create the search plane
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps) {
        // Calculate the convex hull
        ConvexHull convex_hull = examp->CalculateConvexHull(weights, gradient);
        PRINT_DEBUG("Convex hull size == " << convex_hull.size() << endl, 5);
        if(convex_hull.size() == 0) continue;
        // Update the base values
        if(base_stats) base_stats->PlusEquals(*convex_hull[0].second);
        else           base_stats = convex_hull[0].second->Clone();
        PRINT_DEBUG("convex_hull[0]: " << convex_hull[0] << endl, 5);
        // Add all the changed values
        for(int i = 1; i < (int)convex_hull.size(); i++) {
            EvalStatsPtr diff = convex_hull[i].second->Plus(*convex_hull[i-1].second->Times(-1));
            PRINT_DEBUG("convex_hull["<<i<<"]: " << convex_hull[i] << endl, 5);
            if(!diff->IsZero()) {
                map<double,EvalStatsPtr>::iterator it = boundaries.find(convex_hull[i].first.first);
                if(it != boundaries.end()) it->second->PlusEquals(*diff);
                else                       boundaries.insert(make_pair(convex_hull[i].first.first, diff));
            }
        }
    }
    boundaries[DBL_MAX] = base_stats->Times(0);
    // if(boundaries.size() == 0) return make_pair(-DBL_MAX, -DBL_MAX);
    // Find the place with the best score on the plane
    ScoredSpan best_span(Span(-DBL_MAX, -DBL_MAX), base_stats);
    double best_score = -DBL_MAX;
    EvalStatsPtr curr_stats = base_stats->Clone(), zero_stats;
    double last_bound = -DBL_MAX;
    BOOST_FOREACH(const DoublePair & boundary, boundaries) {
        // Find the score at zero. If there is a boundary directly at zero, break ties
        // to the less optimistic side (or gain to the optimistic side)
        if(last_bound <= 0 && boundary.first >= 0)
            zero_stats = curr_stats->Clone();
        // Update the span if it exceeds the previous best and is in the acceptable gradient range
        double curr_score = curr_stats->ConvertToScore();
        if(curr_score > best_score && (last_bound < range.second && boundary.first > range.first)) {
            best_span = ScoredSpan(Span(last_bound, boundary.first), curr_stats->Clone());
            best_score = curr_score;
        }
        PRINT_DEBUG("bef: " << boundary << " curr_stats=" << *curr_stats << endl, 4);
        curr_stats->PlusEquals(*boundary.second);
        PRINT_DEBUG("aft: " << boundary << " curr_stats=" << *curr_stats << endl, 4);
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
    PRINT_DEBUG("0 --> " << zero_stats->ConvertToString() << ", " << middle << " --> " << best_span.second->ConvertToString() << "\t@gradient " << Dict::PrintSparseMap(gradient) << endl, 3);
    return LineSearchResult(middle, *zero_stats, *best_span.second);
}

void TuneMert::Init() {
    // If we have no gradients, find them from the examples
    if(gradients_.size() == 0) {
        set<WordId> potential;
        BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_)
            examp->CountWeights(potential);
        BOOST_FOREACH(WordId val, potential) {
            SparseMap gradient;
            gradient[val] = 1;
            gradients_.push_back(gradient);
        }
    }
}

// Tune new weights using greedy mert until the threshold is exceeded
double TuneMert::RunTuning(SparseMap & weights) {
    // Continue
    double best_score = 0;
    PRINT_DEBUG("Starting MERT Run: " << Dict::PrintSparseMap(weights) << endl, 2);
    while(true) {
        // Initialize the best result
        LineSearchResult best_result;
        SparseMap best_gradient;
        // Perform line search over one gradient
        BOOST_FOREACH(const SparseMap & gradient, gradients_) {
            LineSearchResult result = TuneMert::LineSearch(weights, gradient, examps_);
            // Redo the gain in comparison to the currently saved best score.
            // Given that ties might exist, it is safer to take the gain this way in case
            // a tie results in a change in the best hypothesis.
            result.gain = result.after->ConvertToScore() - best_score;
            if(result.gain > best_result.gain) {
                // Calculate the next score and make sure it is better than the previous
                best_result = result;
                best_gradient = gradient;
            }
        }
        if(best_result.gain <= gain_threshold_) break;
        weights += best_gradient * best_result.pos;
        best_score = best_result.after->ConvertToScore();
        PRINT_DEBUG("Change: " << Dict::PrintSparseMap(best_gradient * best_result.pos) << endl << "After: " << Dict::PrintSparseMap(weights) << endl << best_result.after->ConvertToString() << endl, 2);
    }

    // Normalize so that weights add to 1
    NormalizeL1(weights);

    return best_score;
}
