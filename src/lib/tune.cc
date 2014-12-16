
#include <travatar/tune.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace travatar;

Tune::Tune() : gain_threshold_(0.000001), scale_id_(Dict::WID("__SCALE__")) {
    ranges_[-1] = std::pair<Real,Real>(-REAL_MAX, REAL_MAX);
}

// Find the gradient's range
pair<Real,Real> Tune::FindGradientRange(const SparseMap & weights, WordId feat) {
    RangeMap::const_iterator it = ranges_.find(feat);
    pair<Real,Real> range = (it == ranges_.end() ? ranges_[-1] : it->second);
    SparseMap gradient; gradient[feat] = 1;
    return FindGradientRange(weights, gradient, range);
}

// Current value can be found here
// range(-2,2)
// original(-1)
// change(-1,3)
// gradient -2
// +0.5, -1.5
pair<Real,Real> Tune::FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                pair<Real,Real> range) {
    pair<Real,Real> ret(-REAL_MAX, REAL_MAX);
    BOOST_FOREACH(const SparseMap::value_type & grad, gradient) {
        if(grad.second == 0) continue;
        SparseMap::const_iterator it = weights.find(grad.first);
        Real w = (it != weights.end()) ? it->second : 0.0;
        Real l = (range.first-w)/grad.second;
        Real r = (range.second-w)/grad.second;
        if(l > r) { Real temp = l; l = r; r = temp; }
        ret.first = max(l, ret.first);
        ret.second = min(r, ret.second);
    }
    return ret; 
}

TuningExample & Tune::GetExample(int id) {
    return *examps_[id];
}
