#include <travatar/config-tokenizer-runner.h>
#include <travatar/tokenizer-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTokenizerRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    // train the reorderer
    TokenizerRunner runner;
    runner.Run(conf);
}
