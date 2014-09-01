#ifndef TUNE_MERT_H__
#define TUNE_MERT_H__

#include <travatar/tune.h>
#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <travatar/gradient-xeval.h>
#include <boost/thread.hpp>
#include <vector>
#include <cfloat>
#include <set>

namespace travatar {

class TuningExample;
class TuneMert;
class OutputCollector;
class ThreadPool;

struct LineSearchResult {

    LineSearchResult() :
        pos(0.0), gain(0.0) { }
    LineSearchResult(double p, const EvalStatsPtr & b, const EvalStatsPtr & a) :
        pos(p), before(b->Clone()), after(a->Clone()), gain(a->ConvertToScore()-b->ConvertToScore()) { }
    LineSearchResult(double p, const EvalStats & b, const EvalStats & a) :
        pos(p), before(b.Clone()), after(a.Clone()), gain(a.ConvertToScore()-b.ConvertToScore()) { }

    // The gradient position
    double pos;
    // The total score before
    EvalStatsPtr before;
    // The total score after
    EvalStatsPtr after;
    // The gain between before and after
    double gain;

};

// Performs MERT
class TuneMert : public Tune {

public:

    // **** Static Utility Members ****

    // Perform line search given the current weights and gradient
    static LineSearchResult LineSearch(
      const SparseMap & weights,
      const SparseMap & gradient,
      std::vector<boost::shared_ptr<TuningExample> > & examps,
      std::pair<double,double> range = std::pair<double,double>(-DBL_MAX, DBL_MAX));

    // **** Non-static Members ****
    TuneMert();

    // Tune new weights using MERT
    virtual double RunTuning(SparseMap & weights);

    // Initialize
    virtual void Init(const SparseMap & init_weights);

    void SetDirections(const std::string & str);

    // void UpdateBest(const SparseMap &gradient, const LineSearchResult &result);

protected:
    std::set<WordId> potentials_;
    bool use_coordinate_;
    int num_random_;
    std::vector<double> xeval_scales_;
    GradientXeval xeval_gradient_;

};

}

#endif
