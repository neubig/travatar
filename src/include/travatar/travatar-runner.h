#ifndef TRAVATAR_RUNNER_H__ 
#define TRAVATAR_RUNNER_H__

#include <iostream>
#include <fstream>
#include <travatar/config-travatar-runner.h>
#include <travatar/symbol-set.h>
#include <tr1/unordered_map>

namespace travatar {

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

