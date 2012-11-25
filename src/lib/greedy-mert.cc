#include <travatar/config-greedy-mert.h>
#include <travatar/greedy-mert.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <fstream>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void TravatarRunner::Run(const ConfigTravatarRunner & config) {

    // Open the n-best list
    ifstream nbest(config.GetArg(0));
    if(!nbest)
        THROW_ERROR(config.GetArg(0) << " could not be opened for reading");
    // Open the references
    ifstream ref(config.GetArg(1));
    if(!ref)
        THROW_ERROR(config.GetArg(1) << " could not be opened for reading");
    // Process the references
    HERE


    THROW_ERROR("Not implemented yet");
    
    // Perform MERT
    THROW_ERROR("Not implemented yet");

}
