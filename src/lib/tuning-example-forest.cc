#include <travatar/sparse-map.h>
#include <travatar/tuning-example-forest.h>
#include <travatar/util.h>
#include <travatar/hyper-graph.h>
#include <boost/foreach.hpp>
#include <cfloat>
#include <algorithm>

using namespace std;
using namespace travatar;
using namespace boost;

// ~TuningExampleForest::TuningExampleForest() { }

// Calculate the potential gain for a single example given the current weights
SparseMap TuningExampleForest::CalculatePotentialGain(const SparseMap & weights) const {
    double best_score = 0;
    // TODO: calculate the best score according to this forest
    double gain = oracle_score_ - best_score;
    // Add this to all existing values
    SparseMap ret;
    BOOST_FOREACH(const HyperEdge * edge, forest_->GetEdges())
        BOOST_FOREACH(const SparseMap::value_type & val, edge->GetFeatures())
            ret.insert(make_pair(val.first, gain));
    return ret;
}

// Get the convex hull, which consists of scored spans in order of the span location
ConvexHull TuningExampleForest::CalculateConvexHull(
                        const SparseMap & weights,
                        const SparseMap & gradient) const {
    ConvexHull ret;
    THROW_ERROR("Not implemented yet");
    return ret;
}
