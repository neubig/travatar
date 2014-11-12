#ifndef SYMBOL_SET_H__
#define SYMBOL_SET_H__

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/unordered_map.hpp>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace travatar {

template < class T >
class SymbolSet {

public:

    typedef boost::unordered_map<std::string,T> Map;
    typedef std::vector< std::string* > Vocab;
    typedef std::vector< T > Ids;

protected:
    
    Map map_;
    Vocab vocab_;
    mutable boost::shared_mutex mutex_;
    bool safe_;

public:
    SymbolSet(bool safe = false) : map_(), vocab_(), mutex_(), safe_(safe) { }
    SymbolSet(const SymbolSet & ss) : 
                map_(ss.map_), vocab_(ss.vocab_), mutex_(), safe_(ss.safe_) {
        for(typename Vocab::iterator it = vocab_.begin(); 
                                                it != vocab_.end(); it++) 
            if(*it)
                *it = new std::string(**it);
    }
    ~SymbolSet() {
        for(typename Vocab::iterator it = vocab_.begin(); 
                                            it != vocab_.end(); it++)
            if(*it)
                delete *it;
    }

    const std::vector<std::string*> & GetSymbols() const { return vocab_; }
    const std::string & GetSymbol(T id) const {
        boost::shared_lock< boost::shared_mutex > lock(mutex_);
        return *vocab_[id];
        // return *SafeAccess(vocab_, id);
    }
    T GetIdSafe(const std::string & sym, bool add = false) {
        boost::upgrade_lock< boost::shared_mutex > lock(mutex_);
        typename Map::const_iterator it = map_.find(sym);
        if(it != map_.end())
            return it->second;
        else if(add) {
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            T id;
            id = vocab_.size();
            vocab_.push_back(new std::string(sym));
            map_.insert(std::make_pair(sym,id));
            return id;
        }
        return -1;
    }
    T GetId(const std::string & sym, bool add = false) {
        if(safe_) return GetIdSafe(sym, add);
        typename Map::const_iterator it = map_.find(sym);
        if(it != map_.end())
            return it->second;
        else if(add) {
            T id;
            id = vocab_.size();
            vocab_.push_back(new std::string(sym));
            map_.insert(std::make_pair(sym,id));
            return id;
        }
        return -1;
    }
    T GetId(const std::string & sym) const {
        return const_cast< SymbolSet<T>* >(this)->GetId(sym,false);
    }
    size_t size() const { return vocab_.size(); }
    size_t capacity() const { return vocab_.size(); }
    size_t hashCapacity() const { return map_.size(); }
    
    void ToStream(std::ostream & out) {
        boost::unique_lock< boost::shared_mutex >  lock(mutex_);
        out << vocab_.size() << std::endl;
        for(int i = 0; i < (int)vocab_.size(); i++)
            out << *vocab_[i] << std::endl;
        out << std::endl;
    }
    static SymbolSet<T>* FromStream(std::istream & in) {
        std::string line;
        int size;
        SymbolSet<T> * ret = new SymbolSet<T>;
        getline(in, line); 
        std::istringstream iss(line);
        for(iss >> size; size > 0 && getline(in, line); size--)
            ret->GetId(line, true);
        getline(in,line);
        if(line != "")
            throw std::runtime_error("Expected empty line while reading SymbolSet, but got non-empty line");
        return ret;
    }

    Map & GetMap() { return map_; }

};

}

#endif
