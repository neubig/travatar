#ifndef TRAVATAR_ALIGNMENT_H__ 
#define TRAVATAR_ALIGNMENT_H__

#include <vector>
#include <set>

namespace travatar {

// A string of alignments for a particular sentence
class Alignment {
public:
    // Pair of words in an alignment
    typedef std::pair<int,int> AlignmentPair;

    // Constructor with lengths of the source and target sentence
    Alignment() { }

    // Add a single alignment
    void AddAlignment(const AlignmentPair & al);

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

    std::vector<std::set<int> > GetSrcAlignments() const;

private:

    // Split a string in the form X-Y into an alignment pair
    static AlignmentPair SplitAlignment(const std::string & str);

    std::vector<AlignmentPair> vec_;

};

}

#endif
