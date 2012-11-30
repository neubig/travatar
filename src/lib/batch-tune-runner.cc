#include <travatar/tune-greedy-mert.h>
#include <travatar/config-batch-tune.h>
#include <travatar/batch-tune-runner.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <fstream>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void BatchTuneRunner::Run(const ConfigBatchTune & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Open the n-best list
    ifstream nbest_in(config.GetMainArg(0).c_str());
    if(!nbest_in)
        THROW_ERROR(config.GetMainArg(0) << " could not be opened for reading");
    // Open the references
    ifstream ref_in(config.GetMainArg(1).c_str());
    if(!ref_in)
        THROW_ERROR(config.GetMainArg(1) << " could not be opened for reading");

    // Load the references
    PRINT_DEBUG("Loading references..." << endl, 1);
    string line;
    vector<Sentence> refs;
    int ref_len = 0;
    while(getline(ref_in, line)) {
        Sentence ref = Dict::ParseWords(line);
        refs.push_back(ref);
        ref_len += ref.size();
    }

    // Create the evaluation measure (BLEU for now)
    shared_ptr<EvalMeasure> eval;
    if(config.GetString("eval") == "bleu") {
        eval.reset(new EvalMeasureBleu);
    } else if(config.GetString("eval") == "ribes") {
        eval.reset(new EvalMeasureRibes);
    } else {
        THROW_ERROR("Bad eval measure: " << config.GetString("eval"));
    }

    // Convert the n-best list into example pairs for tuning
    PRINT_DEBUG("Loading nbest..." << endl, 1);
    vector<vector<TuneGreedyMert::ExamplePair> > examps;
    regex threebars(" \\|\\|\\| ");
    while(getline(nbest_in, line)) {
        vector<string> columns;
        algorithm::split_regex(columns, line, threebars);
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        // Get the number
        int id = atoi(columns[0].c_str());
        Sentence hyp = Dict::ParseWords(columns[1]);
        SparseMap feat = Dict::ParseFeatures(columns[3]);
        // Calculate the score
        const Sentence & ref = SafeAccess(refs,id);
        double score = eval->MeasureScore(ref, hyp) * ref.size() / ref_len;
        // Add the example
        if((int)examps.size() <= id) {
            if(id % 100 == 0)
                PRINT_DEBUG(id << ".", 1);
            examps.resize(id+1);
        }
        examps[id].push_back(TuneGreedyMert::ExamplePair(feat, score));
    }
    PRINT_DEBUG(endl, 1);
    
    // Perform MERT
    PRINT_DEBUG("Tuning..." << endl, 1);
    TuneGreedyMert tgm;
    tgm.SetGainThreshold(config.GetDouble("threshold"));
    SparseMap weights;
    tgm.Tune(examps, weights);

    // Print result
    cout << Dict::PrintFeatures(weights) << endl;

}
