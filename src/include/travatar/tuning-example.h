#ifndef TUNING_EXAMPLE_H__
#define TUNING_EXAMPLE_H__

#include <vector>
#include <travatar/sparse-map.h>
#include <travatar/eval-measure.h>
#include <travatar/weights.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

// A span, a span with a score, and a convex hull (collection of scored spans)
typedef std::pair<double,double> Span;
typedef std::pair<Span, EvalStatsPtr> ScoredSpan;
typedef std::vector<ScoredSpan> ConvexHull;

// A pair of features and scores
typedef std::pair<SparseMap, EvalStatsPtr> ExamplePair;

class TuningExample {

public:

    virtual ~TuningExample() { }

    // Calculate the potential gain provided for this particular example
    // given by this weight (will only work for sentence-based measures)
    virtual SparseMap CalculatePotentialGain(const SparseMap & weights) = 0;

    // Add weights for this example
    virtual void CountWeights(SparseMap & weights) = 0;

    // Calculate the convex hull for this example given the current weights
    // and gradients
    virtual ConvexHull CalculateConvexHull(
                                const SparseMap & weights,
                                const SparseMap & gradient) const = 0;

    // Calculate the n-best list giving the current weights
    virtual const std::vector<ExamplePair> & 
                       CalculateNbest(const Weights & weights) = 0;

    // Find the best hypothesis from an example pair.
    virtual const ExamplePair & CalculateModelHypothesis(
                                    const Weights & weights) const = 0;

};

}

#endif
