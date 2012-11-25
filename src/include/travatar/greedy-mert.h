#ifndef GREEDY_MERT_H__ 
#define GREEDY_MERT_H__

namespace travatar {

class ConfigGreedyMert;

class GreedyMert {
public:

    GreedyMert() { }
    ~GreedyMert() { }
    
    // Run the model
    void Run(const ConfigGreedyMert & config);

private:

};

}

#endif

