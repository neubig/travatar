#ifndef CONFIG_TRABATAR_RUNNER_H__
#define CONFIG_TRABATAR_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/util.h>
#include <travatar/config-base.h>

namespace travatar {

class ConfigTravatarRunner : public ConfigBase {

public:

    ConfigTravatarRunner() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 0;

        SetUsage(
"~~~ travatar ~~~\n"
"  by Graham Neubig\n"
"\n"
"A tree to string translator.\n"
"  Usage: travatar < INPUT > OUTPUT\n"
);

        AddConfigEntry("in_format", "penn", "The format of the input (penn/egret)");
        AddConfigEntry("tm_file", "", "Translation model file location");
        AddConfigEntry("tm_storage", "marisa", "Method of storing the rule table (marisa/hash)");
        AddConfigEntry("lm_file", "", "Language model file location");
        AddConfigEntry("weight_file", "", "Weight file location");
        AddConfigEntry("weight_vals", "", "Weight values in format \"name1=val1 name2=val2\", existing features override the file, other features are left unchanged");
        AddConfigEntry("nbest", "1", "The length of the n-best list");
        AddConfigEntry("nbest_out", "", "n-best output file location");
        AddConfigEntry("pop_limit", "100", "The number of pops necessary");
        AddConfigEntry("trace_out", "", "trace output file location");
        AddConfigEntry("binarize", "none", "How to binarize the trees (none/left/right)");


    }
	
};

}

#endif
