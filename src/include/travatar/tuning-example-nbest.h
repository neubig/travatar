#ifndef TUNING_EXAMPLE_NBEST_H__
#define TUNING_EXAMPLE_NBEST_H__

#include <vector>
#include <travatar/sparse-map.h>
#include <travatar/tuning-example.h>

namespace travatar {

class TuningExampleNbest : public TuningExample {

public:

    virtual ~TuningExampleNbest() { }

    // A pair of features and scores
    typedef std::pair<SparseMap,double> ExamplePair;

    // Calculate the gain that could be achieved by each feature
    // for this particular n-best list
    virtual SparseMap CalculatePotentialGain(const SparseMap & weights);

    // Calculate the convex hull for this example given the current weights and gradients
    virtual ConvexHull CalculateConvexHull(
                                const SparseMap & weights,
                                const SparseMap & gradient) const;

    // Add a new hypothesis to the n-best list
    void AddHypothesis(const SparseMap & feats, double score) {
        nbest_.push_back(std::make_pair(feats,score));
    }

protected:

    std::vector<ExamplePair> nbest_;

};

}

#endif
