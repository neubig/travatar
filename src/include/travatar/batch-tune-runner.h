#ifndef BATCH_TUNE_H__ 
#define BATCH_TUNE_H__

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <travatar/sentence.h>
#include <travatar/thread-pool.h>
#include <travatar/sparse-map.h>

namespace travatar {

class ConfigBatchTune;
class TuningExample;
class EvalMeasure;
class Tune;

class BatchTuneRunnerTask : public Task {

public:
    BatchTuneRunnerTask(
        int task_id, const std::string & task_name,
        Tune & tgm, const SparseMap & weights);

    const SparseMap & GetWeights() { return weights_; }
    double GetScore() { return score_; }

    void Run();

private:
    int task_id_;
    std::string task_name_;
    Tune * tgm_;
    SparseMap weights_;
    double score_;

};

class BatchTuneRunner {
public:

    BatchTuneRunner() : ref_len_(0) { }
    ~BatchTuneRunner() { }
    
    // Run the tuner
    void Run(const ConfigBatchTune & config);

    // This function is run when we are doing tuning
    void DoTuning(const ConfigBatchTune & config);

    // This function is run when stat_out is defined, and prints out sentence-by-sentence
    // evaluation statistics
    void CalculateSentenceStats(const ConfigBatchTune & config, const std::string & filename);

private:

    // Load n-best lists or forests
    void LoadNbests(std::istream & sys_in, Tune & tgm, std::istream * stat_in);
    void LoadForests(std::istream & sys_in, Tune & tgm);

    // The evaluation measure to use
    int ref_len_;
    std::vector<Sentence> refs_;
    boost::shared_ptr<EvalMeasure> eval_;

};

}

#endif

