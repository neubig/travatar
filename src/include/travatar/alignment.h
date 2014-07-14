#ifndef TRAVATAR_ALIGNMENT_H__ 
#define TRAVATAR_ALIGNMENT_H__

#include <string>
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
        return this->vec_ == rhs.vec_;
    }
    bool operator!= (const Alignment & rhs) {
        return !(*this == rhs);
    }

    // ------------- Accessors --------------
    const std::set<AlignmentPair> & GetAlignmentVector() const {
        return vec_;
    }

    bool Contains(int i, int j) const {
        return vec_.find(std::make_pair(i, j)) != vec_.end();
    }

    std::vector<std::set<int> > GetSrcAlignments() const;

private:

    // Split a string in the form X-Y into an alignment pair
    static AlignmentPair SplitAlignment(const std::string & str);

    std::set<AlignmentPair> vec_;

};

}

#endif
