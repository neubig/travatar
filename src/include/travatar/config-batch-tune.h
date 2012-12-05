#ifndef CONFIG_BATCH_TUNE_H__
#define CONFIG_BATCH_TUNE_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/config-base.h>

namespace travatar {

class ConfigBatchTune : public ConfigBase {

public:

    ConfigBatchTune() : ConfigBase() {
        minArgs_ = 1;
        maxArgs_ = 1;

// TODO: support multiple references
        SetUsage(
"~~~ batch-tune ~~~\n"
"  by Graham Neubig\n"
"\n"
"Runs batched tuning over n-best lists.\n"
"  Usage: travatar [-nbest NBEST_LIST or -forest FOREST] REFERENCE\n"
);

        AddConfigEntry("nbest", "", "The pointer to a file containing the n-best list of system output");
        AddConfigEntry("forest", "", "The pointer to a file containing translation forests");
        AddConfigEntry("debug", "0", "What level of debugging output to print");
        AddConfigEntry("threshold", "0.0001", "What level of thresholding to use");
        AddConfigEntry("eval", "bleu", "Which evaluation measure to use (bleu/ribes)");
        AddConfigEntry("weight_range", ":", "The range of weights to allow, separated by a colon. Leave blank for infinity. Specific weights can be specified by separating with a pipe and adding a the name followed by equals and the range.");

    }
	
};

}

#endif
