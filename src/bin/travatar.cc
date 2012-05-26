#include <travatar/config-travatar-runner.h>
#include <travatar/travatar-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTravatarRunner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    TravatarRunner runner;
    runner.Run(conf);
}
