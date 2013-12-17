#include <travatar/config-hiero-extractor-runner.h>
#include <travatar/hiero-extractor-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigHieroExtractorRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    HieroExtractorRunner runner;
    runner.Run(conf);
}
