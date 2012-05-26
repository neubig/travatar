#ifndef SYMBOL_SET_H__
#define SYMBOL_SET_H__

#include <vector>
#include <stdexcept>
#include <travatar/util.h>

namespace travatar {

template < class T >
class SymbolSet {

public:

    typedef StringMap<T> Map;
    typedef std::vector< std::string* > Vocab;
    typedef std::vector< T > Ids;

protected:
    
    Map map_;
    Vocab vocab_;
    Ids reuse_;

public:
    SymbolSet() : map_(), vocab_(), reuse_() { }
    SymbolSet(const SymbolSet & ss) : 
                map_(ss.map_), vocab_(ss.vocab_), reuse_(ss.reuse_) {
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
        return *SafeAccess(vocab_, id);
    }
    T GetId(const std::string & sym, bool add = false) {
        typename Map::const_iterator it = map_.find(sym);
        if(it != map_.end())
            return it->second;
        else if(add) {
            T id;
            if(reuse_.size() != 0) {
                id = reuse_.back(); reuse_.pop_back();
                vocab_[id] = new std::string(sym);
            } else {
                id = vocab_.size();
                vocab_.push_back(new std::string(sym));
            }
            map_.insert(MakePair(sym,id));
            return id;
        }
        return -1;
    }
    T GetId(const std::string & sym) const {
        return const_cast< SymbolSet<T>* >(this)->GetId(sym,false);
    }
    size_t size() const { return vocab_.size() - reuse_.size(); }
    size_t capacity() const { return vocab_.size(); }
    size_t hashCapacity() const { return map_.size(); }
    void removeId(const T id) {
        map_.erase(*vocab_[id]);
        delete vocab_[id];
        vocab_[id] = 0;
        reuse_.push_back(id);
    }
    
    void ToStream(std::ostream & out) {
        out << vocab_.size() << std::endl;
        for(int i = 0; i < (int)vocab_.size(); i++)
            out << *vocab_[i] << std::endl;
        out << std::endl;
    }
    static SymbolSet<T>* FromStream(std::istream & in) {
        std::string line;
        int size;
        SymbolSet<T> * ret = new SymbolSet<T>;
        getline(in, line); std::istringstream iss(line);
        for(iss >> size; size > 0 && getline(in, line); size--)
            ret->GetId(line, true);
        GetlineEquals(in, "");
        return ret;
    }

    Map & GetMap() { return map_; }

};

}

#endif
