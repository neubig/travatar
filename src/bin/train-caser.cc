#include <travatar/config-train-caser-runner.h>
#include <travatar/train-caser-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTrainCaserRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    TrainCaserRunner runner;
    runner.Run(conf);
}
