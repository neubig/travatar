#ifndef TUNE_ONLINE_H__
#define TUNE_ONLINE_H__

#include <vector>
#include <cfloat>
#include <boost/thread.hpp>
#include <tr1/unordered_map>
#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <travatar/util.h>
#include <travatar/thread-pool.h>
#include <travatar/eval-measure.h>
#include <travatar/tune.h>

namespace travatar {

class TuningExample;
class TuneOnline;
class OutputCollector;

// Performs online learning
class TuneOnline : public Tune {

public:

    TuneOnline() : shuffle_(true), iters_(20) { }

    // Tune new weights using an online learning algorithm
    virtual double RunTuning(SparseMap & weights);

protected:
    bool shuffle_;
    int iters_;
};

}

#endif
