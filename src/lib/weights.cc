
#include <travatar/weights.h>
#include <boost/foreach.hpp>


using namespace std;
using namespace travatar;

namespace travatar {

double operator*(Weights & lhs, const SparseMap & rhs) {
    double ret = 0;
    BOOST_FOREACH(const SparsePair & val, rhs) {
        ret += val.second * lhs.GetCurrent(val.first);
    }
    return ret;
}

};
