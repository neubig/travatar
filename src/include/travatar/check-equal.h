#ifndef _TRAVATAR_CHECK_EQUAL__
#define _TRAVATAR_CHECK_EQUAL__

#include <travatar/io-util.h>
#include <boost/unordered_map.hpp>
#include <cmath>
#include <map>

#define DEFAULT_ALMOST 1e-6

namespace std {

// Unordered map equality function
template < class K, class T >
inline bool operator==(const boost::unordered_map<K, T> & lhs, 
                       const boost::unordered_map<K, T> & rhs) {
    if(lhs.size() != rhs.size())
        return false;
    for(typename boost::unordered_map<K,T>::const_iterator it = lhs.begin();
        it != lhs.end();
        it++) {
        typename boost::unordered_map<K,T>::const_iterator it2 = rhs.find(it->first);
        if(it2 == rhs.end() || it2->second != it->second)
            return false;
    }
    return true;
}

}

namespace travatar {

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
    const boost::unordered_map<std::string,double> & a,
    const boost::unordered_map<std::string,double> & b) {
    if(a.size() != b.size())
        return false;
    for(boost::unordered_map<std::string,double>::const_iterator it = a.begin(); it != a.end(); it++) {
        boost::unordered_map<std::string,double>::const_iterator it2 = b.find(it->first);
        if(it2 == b.end() || !ApproximateDoubleEquals(it->second, it2->second))
            return false;
    }
    return true;
    
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
           abs(exp[i] - act[i]) > DEFAULT_ALMOST) {
           
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
int CheckMap(const boost::unordered_map<K,V> & exp, const boost::unordered_map<K,V> & act) {
    typedef boost::unordered_map<K,V> MapType;
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
int CheckAlmostMap(const boost::unordered_map<K,double> & exp, const boost::unordered_map<K,double> & act, double diff = DEFAULT_ALMOST) {
    typedef boost::unordered_map<K,double> MapType;
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
    if((act != act) || abs(exp - act) > DEFAULT_ALMOST) {
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

}  // end namespace

#endif
