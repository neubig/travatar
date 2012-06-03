#ifndef TRAVATAR_ALIGNMENT_H__ 
#define TRAVATAR_ALIGNMENT_H__

#include <vector>
#include <set>
#include <travatar/util.h>
#include <boost/foreach.hpp>


namespace travatar {

// A string of alignments for a particular sentence
class Alignment {
public:
    // Pair of words in an alignment
    typedef std::pair<int,int> AlignmentPair;

    // Constructor with lengths of the source and target sentence
    Alignment() { }

    // Add a single alignment
    void AddAlignment(const AlignmentPair & al) {
#ifdef LADER_SAFE
        if(al.first >= len_.first || al.second >= len_.second)
            THROW_ERROR("Out of bounds in AddAlignment: "<< al << ", " << len_);
#endif
        vec_.push_back(al);
    }

    // Convert to and from strings
    std::string ToString() const;
    static Alignment FromString(const std::string & str);

    // Comparators
    bool operator== (const Alignment & rhs) {
        for(int i = 0; i < (int)vec_.size(); i++)
            if(vec_[i] != rhs.vec_[i])
                return false;
        return true;
    }
    bool operator!= (const Alignment & rhs) {
        return !(*this == rhs);
    }

    // ------------- Accessors --------------
    const std::vector<AlignmentPair> & GetAlignmentVector() const {
        return vec_;
    }

    std::vector<std::set<int> > GetSrcAlignments() const {
        std::vector<std::set<int> > ret;
        BOOST_FOREACH(AlignmentPair al, vec_) {
            if((int)ret.size() <= al.first) ret.resize(al.first+1);
            ret[al.first].insert(al.second);
        }
        return ret;
    }

private:

    // Split a string in the form X-Y into an alignment pair
    static AlignmentPair SplitAlignment(const std::string & str);

    std::vector<AlignmentPair> vec_;

};

}

#endif
