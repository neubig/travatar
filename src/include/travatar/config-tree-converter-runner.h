#ifndef CONFIG_TREE_CONVERTER_RUNNER_H__
#define CONFIG_TREE_CONVERTER_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <travatar/config-base.h>

namespace travatar {

class ConfigTreeConverterRunner : public ConfigBase {

public:

    ConfigTreeConverterRunner() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 1;

        SetUsage(
"~~~ tree-converter ~~~\n"
"  by Graham Neubig\n"
"\n"
"Converts parse trees in a number of ways.\n"
"  Usage: tree-converter [SRC_TREES]\n"
);

        AddConfigEntry("input_format", "penn", "The format of the input (penn/json/egret)");
        AddConfigEntry("output_format", "penn", "The format of the output (penn/json/egret/word)");
        AddConfigEntry("split", "", "A regular expression to split words in the tree (e.g. \"-\")");
        AddConfigEntry("compoundsplit", "", "The language model file for use in compound splitting");
        AddConfigEntry("compoundsplit_filler", "", "Optional fillers for compound splitting, e.g. \"e:es\" for German");
        AddConfigEntry("compoundsplit_threshold", "0.01", "Words with unigram probability mass above this threshold will not be split");
        AddConfigEntry("compoundsplit_minchar", "3", "Mininimum required characters in subword for compound splitting");
        AddConfigEntry("binarize", "none", "How to binarize the trees (none/left/right/cky)");
        AddConfigEntry("flatten", "false", "Whether to flatten unary productions");
        AddConfigEntry("debug", "0", "How much debug output to produce");


    }
	
};

}

#endif
