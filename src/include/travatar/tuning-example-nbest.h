#ifndef TUNING_EXAMPLE_NBEST_H__
#define TUNING_EXAMPLE_NBEST_H__

#include <vector>
#include <travatar/eval-measure.h>
#include <travatar/sparse-map.h>
#include <travatar/tuning-example.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

class TuningExampleNbest : public TuningExample {

public:

    virtual ~TuningExampleNbest() { }

    // A pair of features and scores
    typedef std::pair<SparseMap,boost::shared_ptr<EvalStats> > ExamplePair;

    // Calculate the gain that could be achieved by each feature
    // for this particular n-best list
    virtual SparseMap CalculatePotentialGain(const SparseMap & weights);

    // Count weights
    virtual void CountWeights(SparseMap & weights);

    // Calculate the convex hull for this example given the current weights and gradients
    virtual ConvexHull CalculateConvexHull(
                                const SparseMap & weights,
                                const SparseMap & gradient) const;

    // Add a new hypothesis to the n-best list
    void AddHypothesis(const SparseMap & feats, boost::shared_ptr<EvalStats> score) {
        nbest_.push_back(std::make_pair(feats,score));
    }

protected:

    std::vector<ExamplePair> nbest_;

};

}

#endif
