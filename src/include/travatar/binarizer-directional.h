#ifndef BINARIZER_RIGHT_H__
#define BINARIZER_RIGHT_H__

#include <travatar/graph-transformer.h>
#include <travatar/generic-string.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/binarizer.h>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <string>

namespace travatar {

class HyperNode;
class HyperGraph;

// A class for right-binarizing trees
// See:
//  Binarizing Syntax Trees to Improve Syntax-Based Machine Translation Accuracy
//  Wei Wang, Kevin Knight, and Daniel Marcu
class BinarizerDirectional : public Binarizer {

public:

    typedef enum {
        BINARIZE_RIGHT,
        BINARIZE_LEFT
    } Direction;

    typedef boost::unordered_map<GenericString<int>, HyperNode*, GenericHash<GenericString<int> > > SNMap;

    BinarizerDirectional(Direction dir, bool raise_punc = false) 
            : dir_(dir), raise_punc_(raise_punc) {
        punc_labs_.insert(Dict::WID("."));
        punc_labs_.insert(Dict::WID("$."));
        punc_labs_.insert(Dict::WID(","));
        punc_labs_.insert(Dict::WID("$,"));
    }
    virtual ~BinarizerDirectional() { }

    void SetRaisePunc(bool raise_punc) { raise_punc_ = raise_punc; }

    // Binarize the graph to the right
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

protected:

    // Which direction to binarize in
    Direction dir_;

    // Raise punctuation first in right binarization
    bool raise_punc_;

    // A set of labels that indicate punctuation
    boost::unordered_set<WordId> punc_labs_;

    // Find a node indexed by its remaining tails str
    //  hg is the original graph
    //  ret is the new graph
    //  snmap is the mapping from tails to nodes
    //  str is the tails covered by the node
    //  xbar is the symbol to assign if this is a multi-terminal node
    HyperNode * FindIndexedNode(
        const HyperGraph & hg, 
        HyperGraph & ret,
        SNMap & snmap,
        const GenericString<int> & str,
        WordId xbar) const;

};

}

#endif
