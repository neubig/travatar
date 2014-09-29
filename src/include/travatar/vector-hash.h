#ifndef TRAVATAR_VECTOR_HASH__
#define TRAVATAR_VECTOR_HASH__

#include <vector>
#include <boost/foreach.hpp>

namespace travatar {

template <class T>
class VectorHash {
public:
    size_t operator()(const std::vector<T> & x) const {
        size_t hash = 5381;
        for(typename std::vector<T>::const_iterator it = x.begin(); it != x.end(); it++)
            hash = ((hash << 5) + hash) + *it; /* hash * 33 + x[i] */
        return hash;
    }
};

template <class T>
inline std::vector<T> VectorSubstr(const std::vector<T> vec, int start, int len) {
    std::vector<T> ret(len);
    for(int i = 0; i < len; i++)
        ret[i] = vec[i+start];
    return ret;
}

template <class T>
inline std::vector<T> VectorSubstr(const std::vector<T> vec, int start) {
    return VectorSubstr(vec, start, vec.size()-start);
}

template <class T>
inline std::vector<T> VectorAdd(const std::vector<T> & lhs, const std::vector<T> & rhs) {
    std::vector<T> ret(lhs.size() + rhs.size());
    typename std::vector<T>::iterator it = ret.begin();
    BOOST_FOREACH(const T & l, lhs) { *(it++) = l; }
    BOOST_FOREACH(const T & r, rhs) { *(it++) = r; }
    return ret;
}

}

#endif
