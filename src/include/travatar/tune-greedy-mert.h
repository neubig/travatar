#ifndef TUNE_GREEDY_MERT_H__
#define TUNE_GREEDY_MERT_H__

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
class TuneGreedyMert;
class OutputCollector;
class ThreadPool;

struct LineSearchResult {

    LineSearchResult() :
        pos(0.0), gain(0.0) { }
    LineSearchResult(double p, const EvalStatsPtr & b, const EvalStatsPtr & a) :
        pos(p), before(b->Clone()), after(a->Clone()), gain(b->ConvertToScore()-a->ConvertToScore()) { }
    LineSearchResult(double p, const EvalStats & b, const EvalStats & a) :
        pos(p), before(b.Clone()), after(a.Clone()), gain(b.ConvertToScore()-a.ConvertToScore()) { }

    // The gradient position
    double pos;
    // The total score before
    EvalStatsPtr before;
    // The total score after
    EvalStatsPtr after;
    // The gain between before and after
    double gain;

};

// A task
class GreedyMertTask : public Task {
public:
    GreedyMertTask(int id,
                   TuneGreedyMert & tgm,
                   int feature,
                   double potential,
                   OutputCollector * collector) :
        id_(id), tgm_(&tgm), feature_(feature), potential_(potential), collector_(collector) { }
    void Run();
protected:
    int id_;
    TuneGreedyMert * tgm_;
    int feature_;
    double potential_;
    OutputCollector * collector_;
};

// Does MERT in a greedy fashion, at each point testing the value that
// could potentially raise the evaluation by the largest amount. At the
// moment this only works for sentence-decomposable evaluation measures.
class TuneGreedyMert {

public:

    TuneGreedyMert() : gain_threshold_(0.0001), threads_(1), 
                       early_terminate_(false) {
        ranges_[-1] = std::pair<double,double>(-DBL_MAX, DBL_MAX);
    }

    // Tune new weights using greedy mert
    void Tune();

    // Tune pick a single weight to tune and tune it
    // Return the improvement in score
    double TuneOnce();

    // Perform line search given the current weights and gradient
    LineSearchResult LineSearch(
      const SparseMap & weights,
      const SparseMap & gradient,
      std::pair<double,double> range = std::pair<double,double>(-DBL_MAX, DBL_MAX));


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
    int GetThreads() const { return threads_; }
    void SetThreads(int threads) { threads_ = threads; }
    void SetGainThreshold(double thresh) { gain_threshold_ = thresh; }
    double GetGainThreshold() { return gain_threshold_; }
    void SetRange(int id, double min, double max) {
        ranges_[id] = std::pair<double,double>(min,max);
    }
    bool GetEarlyTerminate() const { return early_terminate_; }
    double GetBestGain() const { return best_result_.gain; }
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

    // The examples to use
    std::vector<boost::shared_ptr<TuningExample> > examps_;

    // The best line search result we've found so far and its gradient
    LineSearchResult best_result_;
    SparseMap best_gradient_;
    
    // A Mutex to prevent conflicts in the result update
    boost::mutex result_mutex_;

    // The number of threads to use
    int threads_;

    // Whether to terminate as soon as we have a value that exceeds the
    // gain threshold
    bool early_terminate_;

};

}

#endif
