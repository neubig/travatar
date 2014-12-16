#ifndef TUNING_EXAMPLE_H__
#define TUNING_EXAMPLE_H__

#include <travatar/real.h>
#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>

namespace travatar {

// A span, a span with a score, and a convex hull (collection of scored spans)
class EvalStats;
class Weights;
typedef boost::shared_ptr<EvalStats> EvalStatsPtr;
typedef std::pair<Real,Real> Span;
typedef std::pair<Span, EvalStatsPtr> ScoredSpan;
typedef std::vector<ScoredSpan> ConvexHull;

// A pair of features and scores
typedef std::pair<SparseVector, EvalStatsPtr> ExamplePair;

class TuningExample {

public:

    TuningExample() { }

    virtual ~TuningExample() { }

    // Calculate the potential gain provided for this particular example
    // given by this weight (will only work for sentence-based measures)
    virtual SparseMap CalculatePotentialGain(const SparseMap & weights) = 0;

    // Add weights for this example
    virtual void CountWeights(std::set<WordId> & weights) = 0;

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

};

}

#endif
