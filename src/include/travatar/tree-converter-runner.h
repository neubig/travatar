#ifndef TREE_CONVERTER_RUNNER_H__ 
#define TREE_CONVERTER_RUNNER_H__

namespace travatar {

class ConfigTreeConverterRunner;

// A class to build features for the filterer
class TreeConverterRunner {
public:

    TreeConverterRunner() { }
    ~TreeConverterRunner() { }
    
    // Run the model
    void Run(const ConfigTreeConverterRunner & config);

private:

};

}

#endif

