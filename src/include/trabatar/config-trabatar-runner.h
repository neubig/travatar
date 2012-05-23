#ifndef CONFIG_TRABATAR_RUNNER_H__
#define CONFIG_TRABATAR_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <trabatar/util.h>
#include <trabatar/config-base.h>

namespace trabatar {

class ConfigTrabatarRunner : public ConfigBase {

public:

    ConfigTrabatarRunner() : ConfigBase() {
        minArgs_ = 2;
        maxArgs_ = 2;

        SetUsage(
"~~~ trabatar ~~~\n"
"  by Graham Neubig\n"
"\n"
"Calculates features for a text filterer.\n"
"  Usage: trabatar [INPUT] [FEATURES]\n"
);

        AddConfigEntry("swap", "0.0", "The fraction of sentences to swap");
        AddConfigEntry("use_len_ratio", "true", "Use length ratio features");
        AddConfigEntry("use_model_one", "true", "Use model one features");
        AddConfigEntry("use_alignments", "true", "Use alignment features");
        AddConfigEntry("use_cognates", "true", "Use cognate features");

    }
	
};

}

#endif
