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

    TuningExampleNbest(int factor = 0) : TuningExample(factor) { } 

    virtual ~TuningExampleNbest() { }

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

    // Calculate the n-best list giving the current weights
    virtual const std::vector<ExamplePair> & 
                       CalculateNbest(const Weights & weights) {
        return nbest_;
    }

    // Calculate the n-best list giving the current weights
    virtual const ExamplePair & 
                       CalculateModelHypothesis(Weights & weights) const;
    

protected:

    std::vector<ExamplePair> nbest_;

};

}

#endif
