#include <travatar/config-rescorer-runner.h>
#include <travatar/rescorer-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigRescorer conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    RescorerRunner runner;
    runner.Run(conf);
    return 0;
}
