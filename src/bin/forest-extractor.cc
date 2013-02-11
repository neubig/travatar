#include <travatar/config-forest-extractor-runner.h>
#include <travatar/forest-extractor-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigForestExtractorRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    ForestExtractorRunner runner;
    runner.Run(conf);
}
