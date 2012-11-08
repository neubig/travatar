#ifndef TUNE_GREEDY_MERT_H__
#define TUNE_GREEDY_MERT_H__

#include <travatar/sparse-map.h>
#include <vector>

namespace travatar {

// Does MERT in a greedy fashion, at each point testing the value that
// could potentially raise the evaluation by the largest amount. At the
// moment this only works for sentence-decomposable evaluation measures.
class TuneGreedyMert {

public:

    typedef std::pair<SparseMap,double> ExamplePair;

    TuneGreedyMert() { }

    // Tune new weights using greedy mert
    SparseMap Tune(
               const std::vector<std::vector<ExamplePair> > & examps,
               const SparseMap & init);

    // Calculate the potential gain for a single example given the current
    // weights
    SparseMap CalculatePotentialGain(
                const std::vector<ExamplePair> & examps,
                const SparseMap & weights);

protected:

};

}

#endif
