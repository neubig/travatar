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

    WordSplitter() {
        ignore_.insert(Dict::WID("-LRB-"));
        ignore_.insert(Dict::WID("-RRB-"));
        ignore_.insert(Dict::WID("-lrb-"));
        ignore_.insert(Dict::WID("-rrb-"));
        ignore_.insert(Dict::WID("-LSB-"));
        ignore_.insert(Dict::WID("-RSB-"));
        ignore_.insert(Dict::WID("-lsb-"));
        ignore_.insert(Dict::WID("-rsb-"));
        ignore_.insert(Dict::WID("-LCB-"));
        ignore_.insert(Dict::WID("-RCB-"));
        ignore_.insert(Dict::WID("-lcb-"));
        ignore_.insert(Dict::WID("-rcb-"));
    }

    virtual ~WordSplitter() { }

    // Split words
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    // Split a string
    virtual std::vector<std::string> StringSplit(const std::string & str,
						 const std::string & pad = "") const = 0;

protected:
    std::set<WordId> ignore_;
};

}

#endif
