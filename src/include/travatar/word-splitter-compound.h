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

 WordSplitterCompound(const std::string & lm_file, const std::string & filler = "") : lm_file_(lm_file), filler_(filler), lm_(NULL) { 
    
    // Read Languaga Model file to get unigram statistics
    // Note that allow we only need unigram, kenlm assumes the file is bigram or above
    lm_ =  new lm::ngram::Model(lm_file_.c_str());

  }

    virtual ~WordSplitterCompound() {
      if (lm_) delete lm_;
    }

    // Split a string if the geometric mean of its subparts' unigram frequency 
    // is larger than the whole part's unigram frequency
    // The optional "pad" variable adds a padding to each side of the
    // delimiter to indicate that it should be re-attached
    virtual std::vector<std::string> StringSplit(const std::string & str,
                                        const std::string & pad = "") const;

protected:

    // The filename of the language model
    std::string lm_file_;
    // Fillers to delete during compound splitting e.g. ("es|e" in German "Arbeit+s+tier")
    boost::regex filler_;
    // The language model used for computing splitting decisions
    lm::ngram::Model * lm_; 

};

}

#endif
