#ifndef CONFIG_TRAIN_CASER_RUNNER_H__
#define CONFIG_TRAIN_CASER_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/config-base.h>

namespace travatar {

class ConfigTrainCaserRunner : public ConfigBase {

public:

    ConfigTrainCaserRunner() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 0;

        SetUsage(
"~~~ train-caser ~~~\n"
"  by Graham Neubig\n"
"\n"
"Train a true-casing model from tokenized text.\n"
"  Usage: train-caser < input.txt > model.txt\n"
);

        AddConfigEntry("debug", "0", "How much debug output to produce");


    }
	
};

}

#endif
