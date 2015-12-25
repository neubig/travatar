#ifndef CONFIG_TRAVATAR_RUNNER_H__
#define CONFIG_TRAVATAR_RUNNER_H__

#define ONLINE_TRAINING_ON

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
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
"A tree-to-string machine translation decoder.\n"
"  Usage: travatar -config_file CONFIG < INPUT > OUTPUT\n"
);

        AddConfigEntry("all_unk", "false", "If this is true, translating the word as-is will be an option even when a rule exists");
        AddConfigEntry("binarize", "right", "How to binarize the trees (none/left/right)");
        AddConfigEntry("buffer", "true", "Whether to buffer the output. Turn off if you want file output in real time.");
        AddConfigEntry("chart_limit", "100", "The number of elements in any particular chart cell");
        AddConfigEntry("config_file", "", "The location of the configuration file");
        AddConfigEntry("consider_trg", "false", "Whether lookup t2s consider about target side or not.");
        AddConfigEntry("debug", "1", "What level of debugging output to print");
        AddConfigEntry("delete_unknown", "false", "Delete unknown source word");
        AddConfigEntry("forest_nbest_trim", "0", "Trim the forest so it only includes edges in the n-best");
        AddConfigEntry("forest_out", "", "Forest output file location");
        AddConfigEntry("hiero_span_limit","20", "The span limit of non terminal symbol in hiero translation");
        AddConfigEntry("in_format", "penn", "The format of the input (penn/egret/moses/word)");
        AddConfigEntry("lm_file", "", "Language model file location");
        AddConfigEntry("lm_multi_type", "joint", "How to combine multiple LMs (joint/consec)");
        AddConfigEntry("nbest", "1", "The length of the n-best list");
        AddConfigEntry("nbest_out", "", "n-best output file location");
        AddConfigEntry("nbest_tree", "false", "Print n-best entries with trees used in translations");
        AddConfigEntry("nbest_uniq", "false", "Print only n-best entries with unique target sides");
        AddConfigEntry("pop_limit", "2000", "The number of pops necessary");
        AddConfigEntry("root_symbol", "S", "Root symbol in the rule-table (fsm)");
        AddConfigEntry("search", "inc", "The type of search (Cube Pruning (cp)/Incremental (inc))");
        AddConfigEntry("threads", "1", "The number of threads to use in translation");
        AddConfigEntry("tm_file", "", "Translation model file location");
        AddConfigEntry("tm_storage", "marisa", "Method of storing the rule table (marisa/hash/hiero/fsm)");
        AddConfigEntry("trace_out", "", "trace output file location");
        AddConfigEntry("trg_factors", "1", "The number of types of output to produce");
        AddConfigEntry("unk_symbol", "X", "Unknown word symbol in the rule-table (fsm)");
        AddConfigEntry("weight_vals", "", "Weight values in format \"name1=val1 name2=val2\", existing features override the file, other features are left unchanged");
#ifdef ONLINE_TRAINING_ON
        AddConfigEntry("tune_loss", "bleu", "The evaluation measure to use in tuning (bleu/ribes)");
        AddConfigEntry("tune_ref_files", "", "The reference files to be used for tuning");
        AddConfigEntry("tune_l1_coeff", "0", "The L1 regularization coefficient for tuning");
        AddConfigEntry("tune_step_count", "0", "The number of steps that have been executed previously for tuning");
        AddConfigEntry("tune_step_size", "1", "The size of a single step of weight update");
        AddConfigEntry("tune_update", "none", "How to update the weights after each sentence is translated (none/perceptron)");
        AddConfigEntry("tune_weight_out", "", "Location to print the weight file after done tuning");
        AddConfigEntry("tune_weight_ranges", "", "A space-separated string of MIN|MAX|NAME. When NAME is omitted all non-specified features will be assigned this range.");
#endif


    }
	
};

}

#endif
