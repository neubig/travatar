#ifndef WORD_SPLITTER_REGEX_H__
#define WORD_SPLITTER_REGEX_H__

#include <travatar/graph-transformer.h>
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

class WordSplitterRegex : public WordSplitter {

public:

    WordSplitterRegex(const std::string & profile = "-") : profile_(profile) { }
    virtual ~WordSplitterRegex() { }

    // Split a string with a regex delimiter, and include the delimiter in the
    // result. The optional "pad" variable adds a padding to each side of the
    // delimiter to indicate that it should be re-attached
    virtual std::vector<std::string> StringSplit(const std::string & str,
                                        const std::string & pad = "") const;

protected:
    boost::regex profile_;

};

}

#endif
