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
        AddConfigEntry("attach", "top", "Where to attach null aligned target words (top/none/exhaustive, default top)");
        AddConfigEntry("attach_len", "1", "The maximum length of null segments to attach");
        AddConfigEntry("term_len", "7", "The maximum number of terminals in a rule");
        AddConfigEntry("nonterm_len", "3", "The maximum number of non-terminals in a rule");
        AddConfigEntry("normalize_probs", "true", "Whether or not to normalize counts to probabilities");
        AddConfigEntry("partial_count_thresh", "0.0", "Only print phrases with a partial count greater than this value");


    }
	
};

}

#endif
