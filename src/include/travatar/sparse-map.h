#ifndef SPARSE_MAP_H__
#define SPARSE_MAP_H__

#include <travatar/real.h>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <vector>

namespace travatar {

// Hashtable-based sparse map
typedef std::pair<int, Real> SparsePair;
typedef boost::unordered_map<int, Real> SparseMap;
typedef boost::unordered_map<int, int> SparseIntMap;

bool operator==(const SparseMap & lhs, const SparseMap & rhs);
bool operator!=(const SparseMap & lhs, const SparseMap & rhs);
std::ostream & operator<<(std::ostream & out, const SparseMap & rhs);
SparseMap & operator+=(SparseMap & lhs, const SparseMap & rhs);
SparseMap & operator-=(SparseMap & lhs, const SparseMap & rhs);
SparseMap operator+(const SparseMap & lhs, const SparseMap & rhs);
SparseMap operator-(const SparseMap & lhs, const SparseMap & rhs);
Real operator*(const SparseMap & lhs, const SparseMap & rhs);
SparseMap operator*(const SparseMap & lhs, Real rhs);
void NormalizeL1(SparseMap & map, Real val = 1.0);

// Vector-based sparse map
class SparseVector {
public:
    typedef std::vector<SparsePair> SparseVectorImpl;

    // Direct interfaces to the vector
    inline size_t size() const { return impl_.size(); }
    inline SparseVectorImpl::iterator begin() { return impl_.begin(); }
    inline SparseVectorImpl::iterator end() { return impl_.end(); }
    inline SparseVectorImpl::const_iterator begin() const { return impl_.begin(); }
    inline SparseVectorImpl::const_iterator end() const { return impl_.end(); }

    // Functions for modifying the vector

    // Add a single value
    void Add(int k, Real v);
    // void Add(const std::string & str, Real v);

    // Return the actual vector
    SparseVectorImpl & GetImpl() { return impl_; }
    const SparseVectorImpl & GetImpl() const { return impl_; }

    // Convert to a sparsemap
    SparseMap ToMap();

    SparseVector() { }
    SparseVector(const std::vector<SparsePair> & vec);
protected:
    SparseVectorImpl impl_;
};

bool operator==(const SparseVector & lhs, const SparseVector & rhs);
bool operator!=(const SparseVector & lhs, const SparseVector & rhs);
std::ostream & operator<<(std::ostream & out, const SparseVector & rhs);
SparseVector & operator+=(SparseVector & lhs, const SparseVector & rhs);
SparseVector & operator-=(SparseVector & lhs, const SparseVector & rhs);
SparseVector operator+(const SparseVector & lhs, const SparseVector & rhs);
SparseVector operator-(const SparseVector & lhs, const SparseVector & rhs);
Real operator*(const SparseVector & lhs, const SparseVector & rhs);
SparseVector operator*(const SparseVector & lhs, Real rhs);

Real operator*(const SparseMap & lhs,    const SparseVector & rhs);
SparseMap & operator+=(SparseMap & lhs,    const SparseVector & rhs);
SparseMap & operator-=(SparseMap & lhs,    const SparseVector & rhs);
SparseMap operator+(const SparseMap & lhs, const SparseVector & rhs);
SparseMap operator-(const SparseMap & lhs, const SparseVector & rhs);

}

#endif
