#ifndef BINARIZER_RIGHT_H__
#define BINARIZER_RIGHT_H__

#include <travatar/graph-transformer.h>
#include <travatar/generic-string.h>
#include <travatar/sentence.h>
#include <boost/unordered_map.hpp>
#include <string>

namespace travatar {

class HyperNode;
class HyperGraph;

// A class for right-binarizing trees
// See:
//  Binarizing Syntax Trees to Improve Syntax-Based Machine Translation Accuracy
//  Wei Wang, Kevin Knight, and Daniel Marcu
class BinarizerDirectional : public GraphTransformer {

public:

    typedef enum {
        BINARIZE_RIGHT,
        BINARIZE_LEFT
    } Direction;

    typedef boost::unordered_map<GenericString<int>, HyperNode*, GenericHash<GenericString<int> > > SNMap;

    BinarizerDirectional(Direction dir) : dir_(dir) { }
    virtual ~BinarizerDirectional() { }

    // Binarize the graph to the right
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

protected:

    // Which direction to binarize in
    Direction dir_;

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
