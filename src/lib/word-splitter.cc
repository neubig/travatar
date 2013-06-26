#include <travatar/word-splitter.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

// Binarize the graph to the right
HyperGraph * WordSplitter::TransformGraph(const HyperGraph & hg) const {
    // First copy the graph
    HyperGraph * ret = new HyperGraph(hg);
    THROW_ERROR("WordSplitter not implemented yet");
    return ret;
}
