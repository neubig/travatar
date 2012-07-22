#ifndef CONFIG_FOREST_EXTRACTOR_RUNNER_H__
#define CONFIG_FOREST_EXTRACTOR_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/util.h>
#include <travatar/config-base.h>

namespace travatar {

class ConfigForestExtractorRunner : public ConfigBase {

public:

    ConfigForestExtractorRunner() : ConfigBase() {
        minArgs_ = 3;
        maxArgs_ = 3;

        SetUsage(
"~~~ forest-extractor ~~~\n"
"  by Graham Neubig\n"
"\n"
"Extracts tree-to-string translation rules from trees or forests.\n"
"  Usage: forest-extractor [SRG_TREES] [TRG_TEXT] [ALIGN]\n"
);

        AddConfigEntry("input_format", "penn", "The format of the input (penn/json)");
        AddConfigEntry("binarize", "none", "How to binarize the trees (none/left/right)");
        AddConfigEntry("compose", "1", "How many rules to compose (default 1=no composition)");

    }
	
};

}

#endif
