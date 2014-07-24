#ifndef _TRAVATAR_SOFTMAX__
#define _TRAVATAR_SOFTMAX__

// #include <tr1/unordered_map>
// #include <boost/foreach.hpp>
// #include <boost/shared_ptr.hpp>
// #include <boost/algorithm/string.hpp>
// #include <cmath>
// #include <vector>
// #include <set>
// #include <map>
// #include <string>
// #include <sstream>
// #include <stdexcept>
// #include <algorithm>
// #include <iostream>

namespace travatar {

inline double AddLogProbs(const std::vector<double> & probs) {
    if(probs.size() == 0)
        return -DBL_MAX;
    const unsigned size = probs.size();
    double myMax = std::max(probs[0],probs[size-1]), norm=0;
    for(unsigned i = 0; i < probs.size(); i++)
        norm += exp(probs[i]-myMax);
    return log(norm)+myMax;
}

inline std::vector<double> Softmax(const std::vector<double> & probs) {
    if(probs.size() == 0)
        return std::vector<double>();
    double myMax = probs[0], norm = 0;
    for(unsigned i = 1; i < probs.size(); i++)
        myMax = std::max(myMax, probs[i]);
    std::vector<double> ret(probs.size());
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
