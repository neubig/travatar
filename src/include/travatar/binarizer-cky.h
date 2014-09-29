#ifndef BINARIZER_CKY_H__
#define BINARIZER_CKY_H__

#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <travatar/binarizer.h>
#include <travatar/vector-hash.h>
#include <boost/unordered_map.hpp>
#include <string>

namespace travatar {

class HyperNode;
class HyperGraph;

// A class for CKY binarizing trees
// See:
//  Binarizing Syntax Trees to Improve Syntax-Based Machine Translation Accuracy
//  Wei Wang, Kevin Knight, and Daniel Marcu
class BinarizerCKY : public Binarizer {

public:

    typedef boost::unordered_map<std::vector<int>, HyperNode*, VectorHash<int> > SNMap;

    BinarizerCKY() : max_tails_(7) { }
    virtual ~BinarizerCKY() { }

    // Binarize the graph exhaustively using CKY
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

protected:

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
        const std::vector<int> & str,
        WordId xbar) const;

    // The maximum number of tails that a binarized node can have
    int max_tails_;

};

}

#endif
