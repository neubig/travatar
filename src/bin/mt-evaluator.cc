#include <travatar/config-mt-evaluator-runner.h>
#include <travatar/mt-evaluator-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigMTEvaluatorRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    MTEvaluatorRunner runner;
    runner.Run(conf);
}
