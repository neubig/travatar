#ifndef TUNE_MERT_H__
#define TUNE_MERT_H__

#include <vector>
#include <cfloat>
#include <boost/thread.hpp>
#include <tr1/unordered_map>
#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <travatar/util.h>
#include <travatar/thread-pool.h>
#include <travatar/eval-measure.h>

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
class TuneMert {

public:

    // **** Static Utility Members ****

    // Perform line search given the current weights and gradient
    static LineSearchResult LineSearch(
      const SparseMap & weights,
      const SparseMap & gradient,
      std::vector<boost::shared_ptr<TuningExample> > & examps,
      std::pair<double,double> range = std::pair<double,double>(-DBL_MAX, DBL_MAX));

    // **** Non-static Members ****
    TuneMert() : gain_threshold_(0.000001) {
        ranges_[-1] = std::pair<double,double>(-DBL_MAX, DBL_MAX);
    }

    // Tune weights using MERT mert
    void Tune();

    std::pair<double,double> FindGradientRange(WordId feat);
    std::pair<double,double> FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                std::pair<double,double> range);


    void UpdateBest(const SparseMap &gradient, const LineSearchResult &result);

    void SetExamples(const std::vector<boost::shared_ptr<TuningExample> > & examps) { examps_ = examps; }
    int NumExamples() { return examps_.size(); }
    const SparseMap & GetWeights() const { return weights_; }
    void SetWeights(const SparseMap & weights) { weights_ = weights; }
    void SetGainThreshold(double thresh) { gain_threshold_ = thresh; }
    double GetGainThreshold() { return gain_threshold_; }
    void SetRange(int id, double min, double max) {
        ranges_[id] = std::pair<double,double>(min,max);
    }
    const SparseMap & GetWeights() { return weights_; }
    void AddExample(const boost::shared_ptr<TuningExample> & examp) {
        examps_.push_back(examp);
    }
    TuningExample & GetExample(int id) {
        return *SafeAccess(examps_, id);
    }

protected:

    // A feature must create a gain of more than this to be added
    double gain_threshold_;

    // The range of the weights
    typedef std::tr1::unordered_map<WordId, std::pair<double,double> > RangeMap;
    RangeMap ranges_;

    // The current value of the weights
    SparseMap weights_;
    
    // All weights that can be adjusted
    SparseMap existant_weights_;

    // The examples to use
    std::vector<boost::shared_ptr<TuningExample> > examps_;

};

}

#endif
