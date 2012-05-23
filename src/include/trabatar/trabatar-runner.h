#ifndef TRABATAR_RUNNER_H__ 
#define TRABATAR_RUNNER_H__

#include <iostream>
#include <fstream>
#include <trabatar/config-trabatar-runner.h>
#include <trabatar/symbol-set.h>
#include <tr1/unordered_map>

namespace trabatar {

// A class to build features for the filterer
class TrabatarRunner {
public:

    TrabatarRunner() { }
    ~TrabatarRunner() { }
    
    // Run the model
    void Run(const ConfigTrabatarRunner & config);

private:

};

}

#endif

