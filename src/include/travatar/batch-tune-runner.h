#ifndef BATCH_TUNE_H__ 
#define BATCH_TUNE_H__

namespace travatar {

class ConfigBatchTune;

class BatchTuneRunner {
public:

    BatchTuneRunner() { }
    ~BatchTuneRunner() { }
    
    // Run the model
    void Run(const ConfigBatchTune & config);

private:

};

}

#endif

