#ifndef CONFIG_HIERO_EXTRACTOR_RUNNER_H__
#define CONFIG_HIERO_EXTRACTOR_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/config-base.h>

namespace travatar {

class ConfigHieroExtractorRunner : public ConfigBase {

public:

    ConfigHieroExtractorRunner() : ConfigBase() {
        minArgs_ = 3;
        maxArgs_ = 3;

        SetUsage(
            "~~~ hiero-extractor ~~~\n"
            "  by Philip Arthur\n"
            "\n"
            //"Extracts cfg translation rules from trees or forests.\n"
            //"  Usage: forest-extractor [SRG_TREES] [TRG_FILE] [ALIGN]\n"
        );

        AddConfigEntry("term_len", "10", "The maximum number of terminals in a rule");
        AddConfigEntry("nonterm_len", "3", "The maximum number of non-terminals in a rule");


    }
	
};

}

#endif
