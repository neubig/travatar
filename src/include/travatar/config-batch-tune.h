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
        AddConfigEntry("threads", "1", "The number of threads to use");
        AddConfigEntry("threshold", "1e-6", "Terminate when gains are less than this");
        AddConfigEntry("eval", "bleu", "Which evaluation measure to use (bleu/ribes/ter)");
        AddConfigEntry("algorithm", "mert", "Which tuning algorithm to use (mert)");
        AddConfigEntry("restarts", "20", "The number of random tuning restarts");
        AddConfigEntry("stat_in", "", "Files containing pre-computed statistics for each n-best list");
        AddConfigEntry("stat_out", "", "Set this option to pre-compute statistics for an n-best list");
        AddConfigEntry("weight_in", "", "File of initial weights");
        AddConfigEntry("weight_ranges", "", "A space-separated string of MIN|MAX|NAME. When NAME is omitted all non-specified features will be assigned this range.");

    }
	
};

}

#endif
