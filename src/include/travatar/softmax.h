#ifndef _TRAVATAR_SOFTMAX__
#define _TRAVATAR_SOFTMAX__

#include <travatar/real.h>
#include <cmath>

namespace travatar {

inline Real AddLogProbs(const std::vector<Real> & probs) {
    if(probs.size() == 0)
        return -REAL_MAX;
    const unsigned size = probs.size();
    Real myMax = std::max(probs[0],probs[size-1]), norm=0;
    for(unsigned i = 0; i < probs.size(); i++)
        norm += exp(probs[i]-myMax);
    return log(norm)+myMax;
}

inline std::vector<Real> Softmax(const std::vector<Real> & probs) {
    if(probs.size() == 0)
        return std::vector<Real>();
    Real myMax = probs[0], norm = 0;
    for(unsigned i = 1; i < probs.size(); i++)
        myMax = std::max(myMax, probs[i]);
    std::vector<Real> ret(probs.size());
    for(unsigned i = 0; i < probs.size(); i++) {
        ret[i] = exp(probs[i]-myMax);
        norm += ret[i];
    }
    for(int i = 0; i < (int)ret.size(); i++)
        ret[i] /= norm;
    return ret;
}

}  // end namespace

#endif
