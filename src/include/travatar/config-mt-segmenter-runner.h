#ifndef CONFIG_MT_SEGMENTER_RUNNER_H__
#define CONFIG_MT_SEGMENTER_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <climits>
#include <travatar/config-base.h>

namespace travatar {

class ConfigMTSegmenterRunner : public ConfigBase {

public:

    ConfigMTSegmenterRunner() : ConfigBase() {
        minArgs_ = 1;
        maxArgs_ = 1;

        SetUsage(
"~~~ mt-segmenter ~~~\n"
"  by Graham Neubig\n"
"\n"
"A program to segment the output of an MT system to maximize an evaluation\n"
"measure over a reference\n"
"  Usage: mt-segmenter -ref reference.txt output.txt\n"
);

        AddConfigEntry("ref", "", "A reference file");
        AddConfigEntry("eval", "bleu", "Evaluation measure to be used for the separation");
        AddConfigEntry("debug", "0", "What level of debugging output to print");

    }
	
};

}

#endif
