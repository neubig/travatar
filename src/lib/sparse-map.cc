
#include <travatar/sparse-map.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

namespace travatar {

bool operator==(const SparseMap & lhs, const SparseMap & rhs) {
    // cerr << "Comparing " << lhs << " to " << rhs << endl;
    if(lhs.size() != rhs.size())
        return false;
    BOOST_FOREACH(const SparsePair & kv, lhs) {
        SparseMap::const_iterator it = rhs.find(kv.first);
        if(it == rhs.end() || abs(it->second - kv.second) > 1e-6) return false;
    }
    return true;
}
bool operator!=(const SparseMap & lhs, const SparseMap & rhs) {
    return !(lhs == rhs);
}

std::ostream & operator<<(std::ostream & out, const SparseMap & rhs) {
    out << "{";
    int count = 0;
    BOOST_FOREACH(const SparsePair & kv, rhs) {
        if(kv.second != 0) {
            if(count++ != 0) out << ", ";
            out << "\"" << Dict::WSym(kv.first) << "\": " << kv.second;
        }
    }
    out << "}";
    return out;
}

SparseMap & operator+=(SparseMap & lhs, const SparseMap & rhs) {
    BOOST_FOREACH(const SparsePair & val, rhs)
        if(val.second != 0) {
            lhs[val.first] += val.second;
            if(lhs[val.first] == 0) lhs.erase(val.first);
        }
    return lhs;
}

SparseMap & operator-=(SparseMap & lhs, const SparseMap & rhs) {
    BOOST_FOREACH(const SparsePair & val, rhs)
        if(val.second != 0) {
            lhs[val.first] -= val.second;
            if(lhs[val.first] == 0) lhs.erase(val.first);
        }
    return lhs;
}

SparseMap operator+(const SparseMap & lhs, const SparseMap & rhs) {
    SparseMap ret;
    BOOST_FOREACH(const SparsePair & val, rhs)
        if(val.second != 0)
            ret[val.first] += val.second;
    BOOST_FOREACH(const SparsePair & val, lhs)
        if(val.second != 0) {
            ret[val.first] += val.second;
            if(ret[val.first] == 0) ret.erase(val.first);
        }
    return ret;
}

SparseMap operator-(const SparseMap & lhs, const SparseMap & rhs) {
    SparseMap ret;
    BOOST_FOREACH(const SparsePair & val, lhs)
        if(val.second != 0)
            ret[val.first] += val.second;
    BOOST_FOREACH(const SparsePair & val, rhs) {
        if(val.second != 0) {
            ret[val.first] -= val.second;
            if(ret[val.first] == 0) ret.erase(val.first);
        }
    }
    return ret;
}

double operator*(const SparseMap & lhs, const SparseMap & rhs) {
    double ret = 0;
    if(lhs.size() <= rhs.size()) {
        BOOST_FOREACH(const SparsePair & val, lhs) {
            SparseMap::const_iterator it = rhs.find(val.first);
            if(it != rhs.end())
                ret += val.second * it->second;
        }
    } else {
        BOOST_FOREACH(const SparsePair & val, rhs) {
            SparseMap::const_iterator it = lhs.find(val.first);
            if(it != lhs.end())
                ret += val.second * it->second;
        }
    }
    return ret;
}

SparseMap operator*(const SparseMap & lhs, double rhs) {
    SparseMap ret;
    BOOST_FOREACH(const SparsePair & val, lhs)
        ret[val.first] = val.second * rhs;
    return ret;
}

void NormalizeL1(SparseMap & weights, double denom) {
    double curr = 0;
    BOOST_FOREACH(const SparsePair & val, weights)
        curr += abs(val.second);
    if(curr == 0) return;
    denom = denom/curr;
    BOOST_FOREACH(SparseMap::value_type & val, weights)
        val.second *= denom;
}


SparseVector::SparseVector(const std::vector<SparsePair> & vec) : impl_() {
    if(vec.size() == 0) return;
    impl_ = vec;
    sort(impl_.begin(), impl_.end());
    SparseVectorImpl::iterator itl = impl_.begin(), itr = impl_.begin()+1;
    while(itr != impl_.end()) {
        if(itl->first == itr->first) {
            itl->second += itr->first;
        } else {
            itl++;
            *itl = *itr;
        }
        itr++;
    }
    if(++itl != impl_.end()) {
        impl_.resize(itl - impl_.begin());
    }
}

// Add a single value
void SparseVector::Add(int k, double v) {
    SparseVectorImpl::iterator b = impl_.begin(), e = impl_.end(), n;
    while(1) {
        if(b == e) {
            impl_.insert(e, make_pair(k,v));
            break;
        }
        n = b + (e-b)/2;
        if(n->first < k) {
            b = n+1;
        } else if (n->first > k) {
            e = n;
        } else {
            n->second += v;
            break;
        }
    }
}
void SparseVector::Add(const string & str, double v) {
    Add(Dict::WID(str), v);
}

bool operator==(const SparseVector & lhs, const SparseVector & rhs) {
    const vector<SparsePair> &limpl = lhs.GetImpl(), &rimpl = rhs.GetImpl();
    if(limpl.size() != rimpl.size()) return false;
    for(size_t i = 0; i < limpl.size(); i++)
        if(limpl[i].first != rimpl[i].first || abs(limpl[i].second - rimpl[i].second) > 1e-6)
            return false;
    return true;
}
bool operator!=(const SparseVector & lhs, const SparseVector & rhs) {
    return !(lhs == rhs);
}

std::ostream & operator<<(std::ostream & out, const SparseVector & rhs) {
    out << "{";
    int count = 0;
    BOOST_FOREACH(const SparsePair & kv, rhs.GetImpl()) {
        if(kv.second != 0) {
            if(count++ != 0) out << ", ";
            out << "\"" << Dict::WSym(kv.first) << "\": " << kv.second;
        }
    }
    out << "}";
    return out;
}

SparseVector & operator+=(SparseVector & lhs, const SparseVector & rhs) {
    SparseVector::SparseVectorImpl::iterator itl = lhs.begin();
    SparseVector::SparseVectorImpl::const_iterator itr = rhs.begin();
    SparseVector::SparseVectorImpl & impl = lhs.GetImpl();
    while(itr != rhs.end()) {
        if(itl == lhs.end()) {
            impl.insert(itl, itr, rhs.end());
            break;
        } else if(itl->first == itr->first) {
            itl->second += itr->second;
            itl++; itr++;
        } else if(itl->first < itr->first) {
            itl++;
        } else {
            itl = impl.insert(itl, *itr);
            itr++;
        }
    }
    return lhs;
}

SparseVector & operator-=(SparseVector & lhs, const SparseVector & rhs) {
    SparseVector::SparseVectorImpl::iterator itl = lhs.begin();
    SparseVector::SparseVectorImpl::const_iterator itr = rhs.begin();
    SparseVector::SparseVectorImpl & impl = lhs.GetImpl();
    while(itr != rhs.end()) {
        if(itl == lhs.end()) {
            impl.insert(itl, itr, rhs.end());
            break;
        } else if(itl->first == itr->first) {
            itl->second -= itr->second;
            itl++; itr++;
        } else if(itl->first < itr->first) {
            itl++;
        } else {
            itl = impl.insert(itl, *itr);
            itr++;
        }
    }
    return lhs;
}

SparseVector operator+(const SparseVector & lhs, const SparseVector & rhs) {
    SparseVector::SparseVectorImpl::const_iterator itl = lhs.begin();
    SparseVector::SparseVectorImpl::const_iterator itr = rhs.begin();
    SparseVector ret;
    SparseVector::SparseVectorImpl & impl = ret.GetImpl();
    while(itl != lhs.end() || itr != rhs.end()) {
        if(itr == rhs.end() || itl->first < itr->first) {
            if(itl->second != 0)
                impl.push_back(*itl);
            itl++;
        } else if(itl == lhs.end() || itr->first < itl->first) {
            if(itr->second != 0)
                impl.push_back(*itr);
            itr++;
        } else {
            double val = itl->second + itr->second;
            if(val != 0)
                impl.push_back(make_pair(itl->first, val));
            itl++; itr++;
        }
    }
    return ret;
}

SparseVector operator-(const SparseVector & lhs, const SparseVector & rhs) {
    SparseVector::SparseVectorImpl::const_iterator itl = lhs.begin();
    SparseVector::SparseVectorImpl::const_iterator itr = rhs.begin();
    SparseVector ret;
    SparseVector::SparseVectorImpl & impl = ret.GetImpl();
    while(itl != lhs.end() || itr != rhs.end()) {
        if(itr == rhs.end() || itl->first < itr->first) {
            if(itl->second != 0)
                impl.push_back(*itl);
            itl++;
        } else if(itl == lhs.end() || itr->first < itl->first) {
            if(itr->second != 0)
                impl.push_back(make_pair(itr->first, -itr->second));
            itr++;
        } else {
            double val = itl->second - itr->second;
            if(val != 0)
                impl.push_back(make_pair(itl->first, val));
            itl++; itr++;
        }
    }
    return ret;
}

double operator*(const SparseVector & lhs, const SparseVector & rhs) {
    SparseVector::SparseVectorImpl::const_iterator itl = lhs.begin();
    SparseVector::SparseVectorImpl::const_iterator itr = rhs.begin();
    double ret = 0;
    while(itr != rhs.end() && itl != lhs.end()) {
        if(itl->first == itr->first) {
            ret += itl->second * itr->second;
            itl++; itr++;
        } else if(itl->first < itr->first) {
            itl++;
        } else {
            itr++;
        }
    }
    return ret;
}

SparseVector operator*(const SparseVector & lhs, double rhs) {
    SparseVector ret = lhs;
    BOOST_FOREACH(SparsePair & val, ret.GetImpl())
        val.second *= rhs;
    return ret;
}

double operator*(const SparseMap & lhs, const SparseVector & rhs) {
    double ret = 0;
    BOOST_FOREACH(SparsePair val, rhs.GetImpl()) {
        SparseMap::const_iterator it = lhs.find(val.first);
        if(it != lhs.end())
            ret += val.second * it->second;
    }
    return ret;
}

SparseMap SparseVector::ToMap() {
    SparseMap ret;
    BOOST_FOREACH(SparsePair val, impl_)
        ret.insert(val);
    return ret;
}

}
