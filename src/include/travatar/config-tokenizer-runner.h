#ifndef CONFIG_TOKENIZER_RUNNER_H__
#define CONFIG_TOKENIZER_RUNNER_H__

#include <travatar/config-base.h>
#include <string>

namespace travatar {

class ConfigTokenizerRunner : public ConfigBase {

public:

    ConfigTokenizerRunner() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 0;

        SetUsage(
"~~~ tokenizer ~~~\n"
"  by Graham Neubig\n"
"\n"
"This program tokenizes text one sentence at a time.\n"
"  Usage: tokenize -type penn < input.txt > output.txt\n"
);

        AddConfigEntry("type", "penn", "The tokenizer configuration string (default: penn)");
        AddConfigEntry("debug", "0", "What level of debugging output to print");

    }
	
};

}

#endif
