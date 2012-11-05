#ifndef FOREST_EXTRACTOR_RUNNER_H__ 
#define FOREST_EXTRACTOR_RUNNER_H__

namespace travatar {

class ConfigForestExtractorRunner;

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

