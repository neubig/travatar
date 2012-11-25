#ifndef CONFIG_GREEDY_MERT_H__
#define CONFIG_GREEDY_MERT_H__

#define ONLINE_TRAINING_ON

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/config-base.h>

namespace travatar {

class ConfigGreedyMert : public ConfigBase {

public:

    ConfigGreedyMert() : ConfigBase() {
        minArgs_ = 2;
        maxArgs_ = 2;

// TODO: support multiple references
        SetUsage(
"~~~ travatar ~~~\n"
"  by Graham Neubig\n"
"\n"
"Runs greedy mert.\n"
"  Usage: travatar NBEST_LIST REFERENCE\n"
);

    }
	
};

}

#endif
