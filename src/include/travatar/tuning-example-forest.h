#ifndef TUNING_EXAMPLE_FOREST_H__
#define TUNING_EXAMPLE_FOREST_H__

#include <boost/shared_ptr.hpp>
#include <travatar/tuning-example.h>
#include <travatar/sentence.h>

namespace travatar {

class HyperGraph;

class TuningExampleForest : public TuningExample {

public:

    // Constructor tanking the forest and the reference
    TuningExampleForest(const boost::shared_ptr<HyperGraph> & forest,
                        const Sentence & ref) : 
                            forest_(forest), ref_(ref), oracle_score_(1.0) { }

    virtual ~TuningExampleForest() { }

    // Calculate the gain that could be achieved by each feature
    // for this particular forest (oracle-current best)
    virtual SparseMap CalculatePotentialGain(const SparseMap & weights) const;

    // Calculate the convex hull for this example given the current weights
    // and gradients
    virtual ConvexHull CalculateConvexHull(
                                const SparseMap & weights,
                                const SparseMap & gradient) const;

protected:

    boost::shared_ptr<HyperGraph> forest_;
    Sentence ref_;
    // The score that the best hypothesis in the forest achieves
    double oracle_score_;

};

}

#endif
