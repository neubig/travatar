#ifndef _TRAVATAR_UTIL__
#define _TRAVATAR_UTIL__

#include <cmath>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <tr1/unordered_map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#define TRAVATAR_SAFE

#define THROW_ERROR(msg) do {                   \
    std::ostringstream oss;                     \
    oss << "ERROR: " << msg;                    \
    throw std::runtime_error(oss.str()); }      \
  while (0);

namespace std {

// Unordered map equality function
template < class K, class T >
inline bool operator==(const std::tr1::unordered_map<K, T> & lhs, 
                       const std::tr1::unordered_map<K, T> & rhs) {
    if(lhs.size() != rhs.size())
        return false;
    for(typename std::tr1::unordered_map<K,T>::const_iterator it = lhs.begin();
        it != lhs.end();
        it++) {
        typename std::tr1::unordered_map<K,T>::const_iterator it2 = rhs.find(it->first);
        if(it2 == rhs.end() || it2->second != it->second)
            return false;
    }
    return true;
}

// Output function for pairs
template <class X, class Y>
inline std::ostream& operator << ( std::ostream& out, 
                                   const std::pair< X, boost::shared_ptr<Y> >& rhs )
{
    out << "[" << rhs.first << ", " << *rhs.second << "]";
    return out;
}

// Output function for pairs
template <class X, class Y>
inline std::ostream& operator << ( std::ostream& out, 
                                   const std::pair< X, Y >& rhs )
{
    out << "[" << rhs.first << ", " << rhs.second << "]";
    return out;
}

// Output function for sets
template <class X>
inline std::ostream& operator << ( std::ostream& out, 
                                   const std::set< X >& rhs )
{
    out << "[";
    int val = 0;
    BOOST_FOREACH(const X & x, rhs) {
        if(val++) out << " ";
        out << x;
    }
    out << "]";
    return out;
}

// Input function for pairs
template <class X, class Y>
inline std::istream & operator>> (std::istream & in, std::pair<X,Y>& s) {
    string open, close;
    in >> open >> s.first >> s.second >> close;
    if(open != "<")
        THROW_ERROR("Bad start of pair " << open);
    if(close != "<")
        THROW_ERROR("Bad end of pair " << close);
    return in;
}

// // Less than function for pairs
// template <class X, class Y>
// inline bool operator< ( const std::pair< X, Y >& lhs, const std::pair< X, Y >& rhs ) {
//     return lhs.first < rhs.first || 
//         (lhs.first == rhs.first && lhs.second < rhs.second);
// }

}

namespace travatar {

class GlobalVars {
public:
    static int debug;
};

#define PRINT_DEBUG(msg, lev) do {            \
        if(lev <= GlobalVars::debug)          \
            std::cerr << msg;                 \
        }                                     \
        while (0);

inline bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}

// typedef std::tr1::unordered_map< std::pair<int, int>, double, travatar::PairHash<int> > PairProbMap;
typedef long long WordPairId;
typedef std::tr1::unordered_map< WordPairId, double > PairProbMap;
inline WordPairId HashPair(int x, int y, int yMax) {
    return (WordPairId)x*yMax+y;
}

inline std::vector<std::string> Tokenize(const char *str, char c = ' ') {
    std::vector<std::string> result;
    while(1) {
        const char *begin = str;
        while(*str != c && *str)
            str++;
        result.push_back(std::string(begin, str));
        if(0 == *str++)
            break;
    }
    return result;
}
inline std::vector<std::string> Tokenize(const std::string &str, char c = ' ') {
    return Tokenize(str.c_str(), c);
}

inline void GetlineEquals(std::istream & in, const std::string & str) {
    std::string line;
    std::getline(in, line);
    if(line != str)
        THROW_ERROR("Expected and received inputs differ." << std::endl
                    << "Expect:  " << str << std::endl
                    << "Receive: " << line << std::endl);
}

template <class X>
inline void GetConfigLine(std::istream & in, const std::string & name, X& val) {
    std::string line;
    std::getline(in, line);
    std::istringstream iss(line);
    std::string myName;
    iss >> myName >> val;
    if(name != myName)
        THROW_ERROR("Expected and received inputs differ." << std::endl
                    << "Expect:  " << name << std::endl
                    << "Receive: " << myName << std::endl);
}


inline bool ApproximateDoubleEquals(double a, double b) {
    return (std::abs(a-b) <= std::abs(a)/1000.0);
}

// Check to make sure that two arrays of doubles are approximately equal
inline bool ApproximateDoubleEquals(const std::vector<double> & a, 
                                    const std::vector<double> & b) {
    if(a.size() != b.size())
        return false;
    for(int i = 0; i < (int)a.size(); i++)
        if (!ApproximateDoubleEquals(a[i],b[i]))
            return false;
    return true;
}

inline bool ApproximateDoubleEquals(
    const std::tr1::unordered_map<std::string,double> & a,
    const std::tr1::unordered_map<std::string,double> & b) {
    if(a.size() != b.size())
        return false;
    for(std::tr1::unordered_map<std::string,double>::const_iterator it = a.begin(); it != a.end(); it++) {
        std::tr1::unordered_map<std::string,double>::const_iterator it2 = b.find(it->first);
        if(it2 == b.end() || !ApproximateDoubleEquals(it->second, it2->second))
            return false;
    }
    return true;
    
}

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
inline const T & SafeReference(const std::vector<T*> & ptr) {
    return SafeReference(SafeAccess(ptr));
}

template < class T >
inline const T & SafeReference(const T * ptr) {
#ifdef TRAVATAR_SAFE
    if(!ptr)
        THROW_ERROR("Null pointer access");
#endif
    return *ptr;
}

template <class T>
int CheckEqual(const T & exp, const T & act) {
    if(exp == act) return 1;
    std::cerr << exp << " != " << act << std::endl;
    return 0;
}

template<class T>
int CheckVector(const std::vector<T> & exp, const std::vector<T> & act) {
    int ok = 1;
    for(int i = 0; i < (int)std::max(exp.size(), act.size()); i++) {
        if(i >= (int)exp.size() || 
           i >= (int)act.size() || 
           exp[i] != act[i]) {
           
            ok = 0;
            std::cout << "exp["<<i<<"] != act["<<i<<"] (";
            if(i >= (int)exp.size()) std::cout << "NULL";
            else std::cout << exp[i];
            std::cout <<" != ";
            if(i >= (int)act.size()) std::cout << "NULL"; 
            else std::cout << act[i];
            std::cout << ")" << std::endl;
        }
    }
    return ok;
}

template<class T>
int CheckPtrVector(const std::vector<T*> & exp, const std::vector<T*> & act) {
    int ok = 1;
    for(int i = 0; i < (int)std::max(exp.size(), act.size()); i++) {
        if(i >= (int)exp.size() || 
           i >= (int)act.size() || 
           (exp[i]==NULL) != (act[i]==NULL) ||
           (exp[i]!=NULL && *exp[i] != *act[i])) {
            ok = 0;
            std::cout << "exp["<<i<<"] != act["<<i<<"] (";
            if(i >= (int)exp.size()) std::cout << "OVER";
            else if(exp[i] == NULL) std::cout << "NULL";
            else std::cout << *exp[i];
            std::cout <<" != ";
            if(i >= (int)act.size()) std::cout << "OVER";
            else if(act[i] == NULL) std::cout << "NULL";
            else std::cout << *act[i];
            std::cout << ")" << std::endl;
        }
    }
    return ok;
}
template<class T>
int CheckPtrVector(const std::vector<boost::shared_ptr<T> > & exp, const std::vector<boost::shared_ptr<T> > & act) {
    std::vector<T*> new_exp, new_act;
    BOOST_FOREACH(const boost::shared_ptr<T> & exp_val, exp)
        new_exp.push_back(exp_val.get());
    BOOST_FOREACH(const boost::shared_ptr<T> & act_val, act)
        new_act.push_back(act_val.get());
    return CheckPtrVector(new_exp,new_act);
}

template<class T>
int CheckAlmostVector(const std::vector<T> & exp,
                      const std::vector<T> & act) {
    int ok = 1;
    for(int i = 0; i < (int)std::max(exp.size(), act.size()); i++) {
        if(i >= (int)exp.size() || 
           i >= (int)act.size() || 
           abs(exp[i] - act[i]) > 0.01) {
           
            ok = 0;
            std::cout << "exp["<<i<<"] != act["<<i<<"] (";
            if(i >= (int)exp.size()) std::cout << "NULL";
            else std::cout << exp[i];
            std::cout <<" != ";
            if(i >= (int)act.size()) std::cout << "NULL"; 
            else std::cout << act[i];
            std::cout << ")" << std::endl;
        }
    }
    return ok;
}

template<class K, class V>
int CheckMap(const std::tr1::unordered_map<K,V> & exp, const std::tr1::unordered_map<K,V> & act) {
    typedef std::tr1::unordered_map<K,V> MapType;
    typedef std::pair<K,V> MapPair;
    int ok = 1;
    BOOST_FOREACH(MapPair kv, exp) {
        typename MapType::const_iterator it = act.find(kv.first);
        if(it == act.end()) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << std::endl;
            ok = 0;
        } else if(it->second != kv.second) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != "<<it->second<<")" << std::endl;
            ok = 0;
        }
    }
    BOOST_FOREACH(MapPair kv, act) {
        typename MapType::const_iterator it = exp.find(kv.first);
        if(it == act.end()) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] (NULL != "<<kv.second<<")" << std::endl;
            ok = 0;
        }
    }
    return ok;
}

template<class K, class V>
int CheckMap(const std::map<K,V> & exp, const std::map<K,V> & act) {
    typedef std::map<K,V> MapType;
    typedef std::pair<K,V> MapPair;
    int ok = 1;
    BOOST_FOREACH(MapPair kv, exp) {
        typename MapType::const_iterator it = act.find(kv.first);
        if(it == act.end()) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << std::endl;
            ok = 0;
        } else if(it->second != kv.second) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != "<<it->second<<")" << std::endl;
            ok = 0;
        }
    }
    BOOST_FOREACH(MapPair kv, act) {
        typename MapType::const_iterator it = exp.find(kv.first);
        if(it == act.end()) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] (NULL != "<<kv.second<<")" << std::endl;
            ok = 0;
        }
    }
    return ok;
}

template<class K>
int CheckAlmostMap(const std::tr1::unordered_map<K,double> & exp, const std::tr1::unordered_map<K,double> & act, double diff = 0.01) {
    typedef std::tr1::unordered_map<K,double> MapType;
    typedef std::pair<K,double> MapPair;
    int ok = 1;
    BOOST_FOREACH(MapPair kv, exp) {
        typename MapType::const_iterator it = act.find(kv.first);
        if(it == act.end()) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << std::endl;
            ok = 0;
        } else if(abs(it->second - kv.second) > diff) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != "<<it->second<<")" << std::endl;
            ok = 0;
        }
    }
    BOOST_FOREACH(MapPair kv, act) {
        typename MapType::const_iterator it = exp.find(kv.first);
        if(it == act.end()) {
            std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << std::endl;
            ok = 0;
        }
    }
    return ok;
}

inline int CheckAlmost(double exp, double act) {
    if(abs(exp - act) > 0.01) {
        std::cout << "CheckAlmost: " << exp << " != " << act << std::endl;
        return 0;
    }
    return 1;
}

inline int CheckString(const std::string & exp, const std::string & act) {
    if(exp != act) {
        std::cerr << "CheckString failed" << std::endl << "exp: '"<<exp<<"'"
             <<std::endl<<"act: '"<<act<<"'" <<std::endl;
        for(int i = 0; i < (int)std::min(exp.length(), act.length()); i++)
            if(exp[i] != act[i])
                std::cerr << "exp[" << i << "] '" << exp[i] << "' != act["<<i<<"] '"<<act[i]<<"'" <<std::endl;
        return 0;
    }
    return 1;
}

inline double AddLogProbs(const std::vector<double> & probs) {
    if(probs.size() == 0)
        THROW_ERROR("Cannot add probabilities of size zero");
    const unsigned size = probs.size();
    double myMax = std::max(probs[0],probs[size-1]), norm=0;
    for(unsigned i = 0; i < probs.size(); i++)
        norm += exp(probs[i]-myMax);
    return log(norm)+myMax;
}

inline std::vector<double> Softmax(const std::vector<double> & probs) {
    if(probs.size() == 0)
        THROW_ERROR("Cannot add probabilities of size zero");
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
