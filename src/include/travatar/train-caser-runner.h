#ifndef TRAIN_CASER_RUNNER_H__ 
#define TRAIN_CASER_RUNNER_H__

namespace travatar {

class ConfigTrainCaserRunner;

// A class to build features for the filterer
class TrainCaserRunner {
public:

    TrainCaserRunner() { }
    ~TrainCaserRunner() { }
    
    // Run the model
    void Run(const ConfigTrainCaserRunner & config);

private:

};

}

#endif

