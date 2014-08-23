#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <travatar/tuning-example.h>
#include <travatar/tune-mert.h>
#include <travatar/global-debug.h>
#include <travatar/io-util.h>
#include <travatar/string-util.h>
#include <travatar/dict.h>
#include <travatar/gradient-xeval.h>
#include <travatar/eval-measure.h>
#include <travatar/sparse-map.h>
#include <travatar/output-collector.h>

using namespace std;
using namespace boost;
using namespace travatar;

#define MARGIN 1

TuneMert::TuneMert() : use_coordinate_(true), num_random_(0) { }

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

void TuneMert::Init(const SparseMap & init_weights) {
    if(!potentials_.size()) {
        // Get the coordinate-wise gradients
        BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_)
            examp->CountWeights(potentials_);
        // Initialize the xeval if we are using it
        if(xeval_scales_.size())
            xeval_gradient_.Init(init_weights, examps_);
    }
}

// Tune new weights using greedy mert until the threshold is exceeded
double TuneMert::RunTuning(SparseMap & weights) {
    // Continue
    double best_score = 0;
    PRINT_DEBUG("Starting MERT Run: " << Dict::PrintSparseMap(weights) << endl, 2);
    while(true) {
        // 1) Initialize the best result
        LineSearchResult best_result;
        SparseMap best_gradient;

        // 2) Find the gradients
        vector<SparseMap> gradients(
            (use_coordinate_ ? potentials_.size() : 0)
            + num_random_
            + xeval_scales_.size()
        );
        int curr_grad = 0;
        //  ** Coordinate-wise
        if(use_coordinate_) {
            BOOST_FOREACH(WordId val, potentials_)
                gradients[curr_grad++][val] = 1;
        }
        //  ** Random
        for(int i = 0; i < num_random_; i++) {
            BOOST_FOREACH(WordId val, potentials_)
                gradients[curr_grad][val] = (rand()/(double)RAND_MAX*2 - 1);
            curr_grad++;
        }
        //  ** Xeval
        if(xeval_scales_.size()) {
            double sum = 0.0;
            BOOST_FOREACH(SparseMap::value_type val, weights)
                sum += val.second;
            if(sum == 0.0) sum = 1.0;
            BOOST_FOREACH(double scale, xeval_scales_) {
                weights[xeval_gradient_.GetScaleId()] = scale/sum;
                xeval_gradient_.CalcSparseGradient(weights, gradients[curr_grad]);
                curr_grad++;
            }
        }
        if(weights.find(xeval_gradient_.GetScaleId()) != weights.end())
            weights.erase(xeval_gradient_.GetScaleId());
            // weights.erase(weights.find(xeval_gradient_.GetScaleId()));

        // 3) Perform line search over one gradient at a time
        BOOST_FOREACH(const SparseMap & gradient, gradients) {
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
        // 4) Update based on the gradient
        weights += best_gradient * best_result.pos;
        best_score = best_result.after->ConvertToScore();
        PRINT_DEBUG("Gradient: " << Dict::PrintSparseMap(best_gradient) << endl << "Change: " << Dict::PrintSparseMap(best_gradient * best_result.pos) << endl << "After: " << Dict::PrintSparseMap(weights) << endl << best_result.after->ConvertToString() << endl, 2);
    }

    // Normalize so that weights add to 1
    NormalizeL1(weights);

    return best_score;
}

void TuneMert::SetDirections(const std::string & str) {
    // First reset all
    use_coordinate_ = false;
    num_random_ = 0;
    xeval_scales_.clear();
    BOOST_FOREACH(const std::string & col, Tokenize(str)) {
        vector<string> lr = Tokenize(col, "=");
        if(lr[0] == "coord") {
            use_coordinate_ = true;
        } else if(lr[0] == "rand" && lr.size() == 2) {
            num_random_ = lexical_cast<int>(lr[1]);
        } else if(lr[0] == "xeval" && lr.size() == 2) {
            vector<string> range = Tokenize(lr[1], ":");
            if(range.size() != 3)
                THROW_ERROR("Bad MERT direction specification " << col << ", must specify the min:max:mult for the xeval scale (e.g. 0.01:1000:10)");
            double min_sc = lexical_cast<double>(range[0]);
            double max_sc = lexical_cast<double>(range[1]);
            double mult_sc = lexical_cast<double>(range[2]);
            if(mult_sc <= 1 || min_sc > max_sc)
                THROW_ERROR("Bad MERT direction specification " << col << ", must specify valid range (min <= max) and multiplier (mult > 1) for xeval");
            while(min_sc <= max_sc) {
                xeval_scales_.push_back(min_sc);
                min_sc *= mult_sc;
            }
        } else {
            THROW_ERROR("Bad MERT direction specification " << col << ", valid: \"coord\", \"rand=NUM\", \"xeval=MIN_SCALE:MAX_SCALE:MULTIPLIER\"");
        }
    }
}

