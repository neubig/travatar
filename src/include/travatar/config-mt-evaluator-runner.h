#ifndef CONFIG_MT_EVALUATOR_RUNNER_H__
#define CONFIG_MT_EVALUATOR_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <climits>
#include <travatar/config-base.h>

namespace travatar {

class ConfigMTEvaluatorRunner : public ConfigBase {

public:

    ConfigMTEvaluatorRunner() : ConfigBase() {
        minArgs_ = 1;
        maxArgs_ = INT_MAX;

        SetUsage(
"~~~ mt-evaluator ~~~\n"
"  by Graham Neubig\n"
"\n"
"A program to evaluate MT output.\n"
"  Usage: mt-evaluator -ref reference.txt output1.txt output2.txt\n"
);

        AddConfigEntry("ref", "", "A reference file");
        AddConfigEntry("eval", "bleu,ribes", "Comma separated evaluation types (bleu,bleup1,ribes)");

    }
	
};

}

#endif
