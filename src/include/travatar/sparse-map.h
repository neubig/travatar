#ifndef SPARSE_MAP_H__
#define SPARSE_MAP_H__

#include <tr1/unordered_map>
#include <iostream>
#include <vector>

namespace travatar {

// Hashtable-based sparse map
typedef std::pair<int, double> SparsePair;
typedef std::tr1::unordered_map<int, double> SparseMap;
typedef std::tr1::unordered_map<int, int> SparseIntMap;

bool operator==(const SparseMap & lhs, const SparseMap & rhs);
bool operator!=(const SparseMap & lhs, const SparseMap & rhs);
std::ostream & operator<<(std::ostream & out, const SparseMap & rhs);
SparseMap & operator+=(SparseMap & lhs, const SparseMap & rhs);
SparseMap & operator-=(SparseMap & lhs, const SparseMap & rhs);
SparseMap operator+(const SparseMap & lhs, const SparseMap & rhs);
SparseMap operator-(const SparseMap & lhs, const SparseMap & rhs);
double operator*(const SparseMap & lhs, const SparseMap & rhs);
SparseMap operator*(const SparseMap & lhs, double rhs);
void NormalizeL1(SparseMap & map, double val = 1.0);

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
    void Add(int k, double v);
    void Add(const std::string & str, double v);

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
double operator*(const SparseVector & lhs, const SparseVector & rhs);
SparseVector operator*(const SparseVector & lhs, double rhs);

double operator*(const SparseMap & lhs, const SparseVector & rhs);

}

#endif
