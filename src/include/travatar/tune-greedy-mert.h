#ifndef TUNE_GREEDY_MERT_H__
#define TUNE_GREEDY_MERT_H__

#include <travatar/real.h>
#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <travatar/task.h>
#include <travatar/tune.h>
#include <boost/thread.hpp>
#include <vector>
#include <cfloat>


namespace travatar {

struct LineSearchResult;
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
                   Real potential,
                   OutputCollector * collector) :
        id_(id), tgm_(&tgm), feature_(feature), potential_(potential), collector_(collector) { }
    void Run();
protected:
    int id_;
    TuneGreedyMert * tgm_;
    int feature_;
    Real potential_;
    OutputCollector * collector_;
};

// Does MERT in a greedy fashion, at each point testing the value that
// could potentially raise the evaluation by the largest amount. At the
// moment this only works for sentence-decomposable evaluation measures.
class TuneGreedyMert : public Tune {

public:

    TuneGreedyMert();

    // Tune new weights using greedy mert
    virtual Real RunTuning(SparseMap & weights);

    // Tune pick a single weight to tune and tune it
    // Return the improvement in score
    LineSearchResult TuneOnce(SparseMap & weights);

    void UpdateBest(const SparseMap &gradient, const LineSearchResult &result);

    const SparseMap & GetCurrentWeights() { return curr_weights_; }
    Real GetBestGain(); // { return best_result_.gain; }
    bool GetEarlyTerminate() { return early_terminate_; }
    int GetThreads() const { return threads_; }
    void SetThreads(int threads) { threads_ = threads; }

protected:

    // The best line search result we've found so far and its gradient
    LineSearchResult best_result_;
    SparseMap best_gradient_;

    // Create weights
    SparseMap curr_weights_;
    
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
