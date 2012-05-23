#include <trabatar/config-trabatar-runner.h>
#include <trabatar/trabatar-runner.h>

using namespace trabatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTrabatarRunner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    TrabatarRunner runner;
    runner.Run(conf);
}
