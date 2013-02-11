#include <travatar/config-tree-converter-runner.h>
#include <travatar/tree-converter-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTreeConverterRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    TreeConverterRunner runner;
    runner.Run(conf);
}
