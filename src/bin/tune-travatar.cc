#include <travatar/config-travatar-tuner.h>
#include <travatar/travatar-tuner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTravatarTuner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    TravatarTuner tuner;
    tuner.Run(conf);
    return 0;
}
