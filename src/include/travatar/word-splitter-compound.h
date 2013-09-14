#ifndef WORD_SPLITTER_COMPOUND_H__
#define WORD_SPLITTER_COMPOUND_H__

#include <travatar/graph-transformer.h>
#include <travatar/generic-string.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/word-splitter.h>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <string>
#include <set>

namespace travatar {

class HyperNode;
class HyperGraph;

class WordSplitterCompound : public WordSplitter {

public:

    WordSplitterCompound(const std::string & lm_file, const std::string & filler = "") : lm_file_(lm_file), filler_(filler) { }

    virtual ~WordSplitterCompound() { }

    // Split a string if the geometric mean of its subparts' unigram frequency 
    // is larger than the whole part's unigram frequency
    // The optional "pad" variable adds a padding to each side of the
    // delimiter to indicate that it should be re-attached
    virtual std::vector<std::string> StringSplit(const std::string & str,
                                        const std::string & pad = "") const;

protected:
    std::string lm_file_;
    boost::regex filler_;

};

}

#endif
