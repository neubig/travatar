#include <travatar/config-batch-tune.h>
#include <travatar/batch-tune-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigBatchTune conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    BatchTuneRunner runner;
    runner.Run(conf);
    return 0;
}
