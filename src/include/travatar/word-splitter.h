#ifndef WORD_SPLITTER_RIGHT_H__
#define WORD_SPLITTER_RIGHT_H__

#include <travatar/graph-transformer.h>
#include <travatar/generic-string.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <string>
#include <set>

namespace travatar {

class HyperNode;
class HyperGraph;

class WordSplitter : public GraphTransformer {

public:

    WordSplitter(const std::string & profile = "-") : profile_(profile) {
        ignore_.insert(Dict::WID("-LRB-"));
        ignore_.insert(Dict::WID("-RRB-"));
        ignore_.insert(Dict::WID("-lrb-"));
        ignore_.insert(Dict::WID("-rrb-"));
    }
    virtual ~WordSplitter() { }

    // Split words
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    // Split a string with a regex delimiter, and include the delimiter in the
    // result. The optional "pad" variable adds a padding to each side of the
    // delimiter to indicate that it should be re-attached
    std::vector<std::string> RegexSplit(const std::string & str,
                                        const std::string & pad = "") const;

protected:
    boost::regex profile_;
    std::set<WordId> ignore_;

};

}

#endif
