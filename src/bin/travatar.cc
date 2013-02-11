#include <travatar/config-travatar-runner.h>
#include <travatar/travatar-runner.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTravatarRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv);
    string config_file = conf.GetString("config_file");
    if(config_file.length() != 0)
        conf.LoadConfig(config_file);
    conf.LoadConfig(argc, argv);
    // train the reorderer
    TravatarRunner runner;
    runner.Run(conf);
    return 0;
}
