#ifndef FOREST_EXTRACTOR_RUNNER_H__ 
#define FOREST_EXTRACTOR_RUNNER_H__

#include <iostream>
#include <fstream>
#include <travatar/config-forest-extractor-runner.h>
#include <travatar/symbol-set.h>
#include <tr1/unordered_map>

namespace travatar {

// A class to build features for the filterer
class ForestExtractorRunner {
public:

    ForestExtractorRunner() { }
    ~ForestExtractorRunner() { }
    
    // Run the model
    void Run(const ConfigForestExtractorRunner & config);

private:

};

}

#endif

