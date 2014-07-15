#ifndef TUNING_EXAMPLE_H__
#define TUNING_EXAMPLE_H__

#include <travatar/sparse-map.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace travatar {

// A span, a span with a score, and a convex hull (collection of scored spans)
class EvalStats;
class Weights;
typedef boost::shared_ptr<EvalStats> EvalStatsPtr;
typedef std::pair<double,double> Span;
typedef std::pair<Span, EvalStatsPtr> ScoredSpan;
typedef std::vector<ScoredSpan> ConvexHull;

// A pair of features and scores
typedef std::pair<SparseMap, EvalStatsPtr> ExamplePair;

class TuningExample {

public:

    TuningExample(int factor = 0) : factor_(factor) { }

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
                                    Weights & weights) const = 0;

protected:

    // The target factor to be evaluated
    int factor_;

};

}

#endif
