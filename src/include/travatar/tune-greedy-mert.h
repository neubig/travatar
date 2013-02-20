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
#include <travatar/tune.h>
#include <travatar/tune-mert.h>

namespace travatar {

class TuningExample;
class TuneGreedyMert;
class OutputCollector;
class ThreadPool;

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
class TuneGreedyMert : public Tune {

public:

    TuneGreedyMert() : Tune(),
                       early_terminate_(false) { }

    // Tune new weights using greedy mert
    virtual void RunTuning();

    // Tune pick a single weight to tune and tune it
    // Return the improvement in score
    double TuneOnce();

    std::pair<double,double> FindGradientRange(WordId feat);
    std::pair<double,double> FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                std::pair<double,double> range);


    void UpdateBest(const SparseMap &gradient, const LineSearchResult &result);

    double GetBestGain() { return best_result_.gain; }
    bool GetEarlyTerminate() { return early_terminate_; }
    int GetThreads() const { return threads_; }
    void SetThreads(int threads) { threads_ = threads; }

protected:

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
