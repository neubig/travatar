#ifndef TRAVATAR_RUNNER_H__ 
#define TRAVATAR_RUNNER_H__

namespace travatar {

class ConfigTravatarRunner;

// A class to build features for the filterer
class TravatarRunner {
public:

    TravatarRunner() { }
    ~TravatarRunner() { }
    
    // Run the model
    void Run(const ConfigTravatarRunner & config);

private:

};

}

#endif

