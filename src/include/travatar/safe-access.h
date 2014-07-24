#ifndef _TRAVATAR_SAFE_ACCESS__
#define _TRAVATAR_SAFE_ACCESS__

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
#include <travatar/global-debug.h>

#define TRAVATAR_SAFE

namespace travatar {

// Perform safe access to a vector
template < class T >
inline const T & SafeAccess(const std::vector<T> & vec, int idx) {
#ifdef TRAVATAR_SAFE
    if(idx < 0 || idx >= (int)vec.size())
        THROW_ERROR("Out of bound access size="<<vec.size()<<", idx="<<idx);
#endif
    return vec[idx];
}

// Perform safe access to a vector
template < class T >
inline T & SafeAccess(std::vector<T> & vec, int idx) {
#ifdef TRAVATAR_SAFE
    if(idx < 0 || idx >= (int)vec.size())
        THROW_ERROR("Out of bound access size="<<vec.size()<<", idx="<<idx);
#endif
    return vec[idx];
}

template < class T >
inline const T & SafeReference(const T * ptr) {
#ifdef TRAVATAR_SAFE
    if(!ptr)
        THROW_ERROR("Null pointer access");
#endif
    return *ptr;
}

template < class T >
inline const T & SafeReference(const std::vector<T*> & ptr) {
    return SafeReference(SafeAccess(ptr));
}

}  // end namespace

#endif
