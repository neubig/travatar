#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <travatar/config-batch-tune.h>
#include <travatar/batch-tune-runner.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/tune-greedy-mert.h>
#include <travatar/tuning-example-nbest.h>

using namespace travatar;
using namespace std;
using namespace boost;

void BatchTuneRunner::LoadNbests(istream & sys_in, 
                                 vector<shared_ptr<TuningExample> > & examps) {
    string line;
    regex threebars(" \\|\\|\\| ");
    while(getline(sys_in, line)) {
        vector<string> columns;
        algorithm::split_regex(columns, line, threebars);
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        // Get the number
        int id = atoi(columns[0].c_str());
        Sentence hyp = Dict::ParseWords(columns[1]);
        SparseMap feat = Dict::ParseFeatures(columns[3]);
        // Calculate the score
        const Sentence & ref = SafeAccess(refs_,id);
        double score = eval_->MeasureScore(ref, hyp, id) * ref.size() / ref_len_;
        // Add the example
        while((int)examps.size() <= id) {
            if(id % 100 == 0)
                PRINT_DEBUG(id << ".", 1);
            examps.push_back(shared_ptr<TuningExample>(new TuningExampleNbest()));
        }
        ((TuningExampleNbest&)*examps[id]).AddHypothesis(feat, score);
    }
    PRINT_DEBUG(endl, 1);
}

void BatchTuneRunner::LoadForests(istream & sys_in, 
                                  vector<shared_ptr<TuningExample> > & examps) {
    THROW_ERROR("Not implemented yet");
}

// Run the model
void BatchTuneRunner::Run(const ConfigBatchTune & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Open the references
    ifstream ref_in(config.GetMainArg(0).c_str());
    if(!ref_in)
        THROW_ERROR(config.GetMainArg(0) << " could not be opened for reading");

    // Open the n-best list if it exists
    bool use_nbest = config.GetString("nbest") != "";
    bool use_forest = config.GetString("forest") != "";
    if(!(use_nbest ^ use_forest))
        THROW_ERROR("Must specify either -nbest or -forest and not both");
    string sys_file = use_nbest ? 
                      config.GetString("nbest") : 
                      config.GetString("forest");
    ifstream sys_in(sys_file.c_str());
    if(!sys_in)
        THROW_ERROR(sys_file << " could not be opened for reading");

    // Load the references
    PRINT_DEBUG("Loading references..." << endl, 1);
    string line;
    while(getline(ref_in, line)) {
        Sentence ref = Dict::ParseWords(line);
        refs_.push_back(ref);
        ref_len_ += ref.size();
    }

    // Create the evaluation measure (BLEU for now)
    if(config.GetString("eval") == "bleu") {
        eval_.reset(new EvalMeasureBleu);
    } else if(config.GetString("eval") == "ribes") {
        eval_.reset(new EvalMeasureRibes);
    } else {
        THROW_ERROR("Bad eval measure: " << config.GetString("eval"));
    }

    // Convert the n-best lists or forests into example pairs for tuning
    PRINT_DEBUG("Loading system output..." << endl, 1);
    vector<shared_ptr<TuningExample> > examps;
    if(use_nbest)
        LoadNbests(sys_in, examps);
    else
        LoadForests(sys_in, examps);
    
    // Perform MERT
    PRINT_DEBUG("Tuning..." << endl, 1);
    TuneGreedyMert tgm;
    tgm.SetGainThreshold(config.GetDouble("threshold"));
    SparseMap weights;
    tgm.Tune(examps, weights);

    // Print result
    cout << Dict::PrintFeatures(weights) << endl;

}
