
#include <travatar/tune.h>

using namespace std;
using namespace travatar;

// Find the gradient's range
pair<double,double> Tune::FindGradientRange(const SparseMap & weights, WordId feat) {
    RangeMap::const_iterator it = ranges_.find(feat);
    pair<double,double> range = (it == ranges_.end() ? ranges_[-1] : it->second);
    SparseMap gradient; gradient[feat] = 1;
    return FindGradientRange(weights, gradient, range);
}

// Current value can be found here
// range(-2,2)
// original(-1)
// change(-1,3)
// gradient -2
// +0.5, -1.5
pair<double,double> Tune::FindGradientRange(
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
