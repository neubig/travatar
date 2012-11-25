#include <travatar/config-greedy-mert.h>
#include <travatar/greedy-mert.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigGreedyMert conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    // GreedyMert runner;
    // runner.Run(conf);
    return 0;
}
