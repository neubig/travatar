#ifndef HIERO_EXTRACTOR_RUNNER_H__ 
#define HIERO_EXTRACTOR_RUNNER_H__

namespace travatar {

class ConfigHieroExtractorRunner;

// A class to build features for the filterer
class HieroExtractorRunner {
public:

    HieroExtractorRunner() { }
    ~HieroExtractorRunner() { }
    
    // Run the model
    void Run(const ConfigHieroExtractorRunner & config);
    
private:
    void IsSane(const ConfigHieroExtractorRunner & config);
    std::string PrintAlignment (const std::vector< std::pair<int,int> > & alignments);
};

}

#endif

