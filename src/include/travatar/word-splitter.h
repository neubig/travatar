#ifndef WORD_SPLITTER_RIGHT_H__
#define WORD_SPLITTER_RIGHT_H__

#include <travatar/graph-transformer.h>
#include <travatar/generic-string.h>
#include <travatar/sentence.h>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <string>

namespace travatar {

class HyperNode;
class HyperGraph;

class WordSplitter : public GraphTransformer {

public:

    WordSplitter(const std::string & profile = "-") : profile_(profile) { }
    virtual ~WordSplitter() { }

    // Split words
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

protected:
    boost::regex profile_;

};

}

#endif
