#include <travatar/config-mt-segmenter-runner.h>
#include <travatar/mt-segmenter-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigMTSegmenterRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    MTSegmenterRunner runner;
    runner.Run(conf);
}
