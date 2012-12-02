#ifndef TUNE_GREEDY_MERT_H__
#define TUNE_GREEDY_MERT_H__

#include <travatar/sparse-map.h>
#include <vector>
#include <cfloat>

namespace travatar {

// Does MERT in a greedy fashion, at each point testing the value that
// could potentially raise the evaluation by the largest amount. At the
// moment this only works for sentence-decomposable evaluation measures.
class TuneGreedyMert {

public:

    typedef std::pair<SparseMap,double> ExamplePair;

    TuneGreedyMert() : gain_threshold_(0.0001), range_(0, DBL_MAX) { }

    // Tune new weights using greedy mert
    void Tune(
        const std::vector<std::vector<ExamplePair> > & examps,
        SparseMap & weights);

    // Tune pick a single weight to tune and tune it
    // Return the improvement in score
    double TuneOnce(
        const std::vector<std::vector<ExamplePair> > & examps,
        SparseMap & weights);

    // Calculate the potential gain for a single example given the current
    // weights
    SparseMap CalculatePotentialGain(
                const std::vector<ExamplePair> & examps,
                const SparseMap & weights);


    typedef std::pair<double,double> Span;
    typedef std::pair<Span, double> ScoredSpan;

    // Get the convex hull, which consists of scored spans in order of the span location
    std::vector<ScoredSpan> CalculateConvexHull(
                       const std::vector<ExamplePair> & examps, 
                       const SparseMap & weights,
                       const SparseMap & gradient);

    // Perform line search given the current weights and gradient
    std::pair<double,double> LineSearch(
                const std::vector<std::vector<ExamplePair> > & examps, 
                const SparseMap & weights,
                const SparseMap & gradient,
                std::pair<double,double> range = std::pair<double,double>(-DBL_MAX, DBL_MAX));


    std::pair<double,double> FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                std::pair<double,double> range);

    void SetGainThreshold(double thresh) { gain_threshold_ = thresh; }
    double GetGainThreshold() { return gain_threshold_; }

protected:

    // A feature must create a gain of more than this to be added
    double gain_threshold_;
    // The range of the weights
    std::pair<double,double> range_;

};

}

#endif
