#ifndef CONFIG_TRAVATAR_TRAINER_H__
#define CONFIG_TRAVATAR_TRAINER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/util.h>
#include <travatar/config-base.h>

namespace travatar {

class ConfigTravatarTrainer : public ConfigBase {

public:

    ConfigTravatarTrainer() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 0;

        SetUsage(
"~~~ train-travatar ~~~\n"
"  by Graham Neubig\n"
"\n"
"A training program for travatar.\n"
"  Usage: train-travatar [OPTIONS]\n"
);

    }
	
};

}

#endif
