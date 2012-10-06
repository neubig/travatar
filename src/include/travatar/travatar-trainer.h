#ifndef TRAVATAR_TRAINER_H__ 
#define TRAVATAR_TRAINER_H__

#include <iostream>
#include <fstream>
#include <travatar/config-travatar-runner.h>
#include <travatar/symbol-set.h>
#include <tr1/unordered_map>

namespace travatar {

// Trains a travatar model
class TravatarTrainer {
public:

    TravatarTrainer() { }
    ~TravatarTrainer() { }
    
    // Run the trainer
    void Run(const ConfigTravatarTrainer & config);

    // Final step, write the post-tuning 

private:

};

}

#endif

