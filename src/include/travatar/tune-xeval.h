#ifndef TUNE_XEVAL_H__
#define TUNE_XEVAL_H__

#include <vector>
#include <cfloat>
#include <boost/thread.hpp>
#include <tr1/unordered_map>
#include <travatar/sparse-map.h>
#include <travatar/tune.h>

namespace travatar {

// Performs gradient ascent to maximize the expectation of the eval measure
// This is a generalization of the method proposed in:
//   Rosti, A.-V., Zhang, B., Matsoukas, S. and Schwartz, R.
//   BBN System Description for WMT10 System Combination Task
//
// The implementation here follows the description (in Japanese)
//   機械翻訳 7.2.4章 (ベイズリスク最小化)

class TuneXeval : public Tune {

public:

    TuneXeval() { }

    // Tune new weights to maximize the expectation of the evaluation measure
    virtual double RunTuning(SparseMap & weights);

    // Initialize
    virtual void Init();

protected:
    

};

}

#endif
