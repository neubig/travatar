#include <travatar/config-travatar-runner.h>
#include <travatar/travatar-runner.h>
#include <travatar/global-debug.h>

using namespace travatar;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigTravatarRunner conf;
    vector<string> args = conf.LoadConfig(argc,argv,false);
    string config_file = conf.GetString("config_file");
    if(config_file.length() == 0)
        THROW_ERROR("Must specify configuration using -config_file");
    conf.LoadConfig(config_file);
    conf.LoadConfig(argc, argv, true);
    // train the reorderer
    TravatarRunner runner;
    runner.Run(conf);
    return 0;
}
