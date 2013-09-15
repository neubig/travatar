#ifndef WORD_SPLITTER_COMPOUND_H__
#define WORD_SPLITTER_COMPOUND_H__

#include <travatar/graph-transformer.h>
#include <travatar/generic-string.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/word-splitter.h>
#include <lm/model.hh>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <string>
#include <set>

namespace travatar {

class HyperNode;
class HyperGraph;

class WordSplitterCompound : public WordSplitter {

public:

    struct SplitStatistics {int words, candidates, splits;};

    WordSplitterCompound(const std::string & lm_file, unsigned int min_char, float threshold, const std::string & filler);

    virtual ~WordSplitterCompound(); 

    // Split a string if the geometric mean of its subparts' unigram frequency 
    // is larger than the whole part's unigram frequency
    // The optional "pad" variable adds a padding to each side of the
    // delimiter to indicate that it should be re-attached
    virtual std::vector<std::string> StringSplit(const std::string & str,
                                        const std::string & pad = "") const;

protected:

    // Fillers to delete during splitting e.g. ("es:s" in German "Arbeit+s+tier")
    std::vector<std::string> fillers_;
    // The language model used for computing splitting decisions
    lm::ngram::Model * lm_; 
    // Threshold for considering as candidate for splitting
    float logprob_threshold_;
    // Subword should be at least of min_char_ characters (including filler length)
    unsigned int min_char_;
    // Accumulated statistics on #words processed and #splitted over the course of this object's lifetime. 
    SplitStatistics * stat_;
};

}

#endif
