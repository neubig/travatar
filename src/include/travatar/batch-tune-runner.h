#ifndef BATCH_TUNE_H__ 
#define BATCH_TUNE_H__

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <travatar/sentence.h>

namespace travatar {

class ConfigBatchTune;
class TuningExample;
class EvalMeasure;
class TuneGreedyMert;

class BatchTuneRunner {
public:

    BatchTuneRunner() : ref_len_(0) { }
    ~BatchTuneRunner() { }
    
    // Run the model
    void Run(const ConfigBatchTune & config);

private:

    // Load n-best lists or forests
    void LoadNbests(std::istream & sys_in, TuneGreedyMert & tgm);
    void LoadForests(std::istream & sys_in, TuneGreedyMert & tgm);

    // The evaluation measure to use
    int ref_len_;
    std::vector<Sentence> refs_;
    boost::shared_ptr<EvalMeasure> eval_;

};

}

#endif

