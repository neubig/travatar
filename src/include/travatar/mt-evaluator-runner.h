#ifndef MT_EVALUATOR_RUNNER_H__ 
#define MT_EVALUATOR_RUNNER_H__

namespace travatar {

class ConfigMTEvaluatorRunner;

// A class to build features for the filterer
class MTEvaluatorRunner {
public:

    MTEvaluatorRunner() { }
    ~MTEvaluatorRunner() { }
    
    // Run the model
    void Run(const ConfigMTEvaluatorRunner & config);

private:

};

}

#endif

