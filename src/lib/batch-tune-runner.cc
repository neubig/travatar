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
#include <travatar/tuning-example-forest.h>
#include <travatar/hyper-graph.h>
#include <travatar/tree-io.h>
#include <travatar/output-collector.h>

using namespace travatar;
using namespace std;
using namespace boost;

void BatchTuneRunner::LoadNbests(istream & sys_in, Tune & tgm) {
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
        shared_ptr<EvalStats> stats = eval_->CalculateStats(ref, hyp, id);
        // Add the example
        while((int)tgm.NumExamples() <= id) {
            if(id % 100 == 0)
                PRINT_DEBUG(id << ".", 1);
            tgm.AddExample(shared_ptr<TuningExample>(new TuningExampleNbest()));
        }
        ((TuningExampleNbest&)tgm.GetExample(id)).AddHypothesis(feat, stats);
    }
    PRINT_DEBUG(endl, 1);
}

void BatchTuneRunner::LoadForests(istream & sys_in, Tune & tgm) {
    JSONTreeIO io;
    HyperGraph * curr_ptr;
    int id = 0;
    bool normalize_len = false;
    while((curr_ptr = io.ReadTree(sys_in)) != NULL) {
        if(id % 100 == 0)
            PRINT_DEBUG(id << ".", 1);
        PRINT_DEBUG("Loading line " << id << endl, 1);
        const Sentence & ref = SafeAccess(refs_,id);
        // Add the example
        if((int)tgm.NumExamples() <= id) {
            double norm = (normalize_len ? ref.size() / (double)ref_len_ : 1.0);
            tgm.AddExample(
                shared_ptr<TuningExample>(
                    new TuningExampleForest(
                        eval_.get(),
                        ref, id, norm)));
        }
        ((TuningExampleForest&)tgm.GetExample(id)).AddHypothesis(shared_ptr<HyperGraph>(curr_ptr));
        id++;
    }
}

// Run the model
void BatchTuneRunner::Run(const ConfigBatchTune & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Load the features from the weight file
    SparseMap weights;
    if(config.GetString("weight_in") != "") {
        cerr << "Reading weight file from "<<config.GetString("weight_in")<<"..." << endl;
        ifstream weight_in(config.GetString("weight_in").c_str());
        if(!weight_in)
            THROW_ERROR("Could not find weights: " << config.GetString("weight_in"));
        weights = Dict::ParseFeatures(weight_in);
        weight_in.close();
    }

    // Open the references
    ifstream ref_in(config.GetMainArg(0).c_str());
    if(!ref_in)
        THROW_ERROR(config.GetMainArg(0) << " could not be opened for reading");

    // Open the n-best list if it exists
    bool use_nbest = config.GetString("nbest") != "";
    bool use_forest = config.GetString("forest") != "";
    if(!(use_nbest ^ use_forest))
        THROW_ERROR("Must specify either -nbest or -forest and not both");

    // Load the references
    PRINT_DEBUG("Loading references..." << endl, 1);
    string line;
    while(getline(ref_in, line)) {
        Sentence ref = Dict::ParseWords(line);
        refs_.push_back(ref);
        ref_len_ += ref.size();
    }

    // Create the evaluation measure
    if(config.GetString("eval") == "bleu") {
        eval_.reset(new EvalMeasureBleu);
    } else if(config.GetString("eval") == "ribes") {
        eval_.reset(new EvalMeasureRibes);
    } else {
        THROW_ERROR("Bad eval measure: " << config.GetString("eval"));
    }

    // Chose the MERT
    shared_ptr<Tune> tgm;
    if(config.GetString("algorithm") == "mert") {
        tgm.reset(new TuneMert);
    } else if(config.GetString("algorithm") == "greedy-mert") {
        tgm.reset(new TuneGreedyMert);
        ((TuneGreedyMert&)*tgm).SetThreads(config.GetInt("threads"));
    }

    // Convert the n-best lists or forests into example pairs for tuning
    PRINT_DEBUG("Loading system output..." << endl, 1);
    string sys_file = use_nbest ? config.GetString("nbest") : config.GetString("forest");
    vector<string> sys_files;
    boost::split(sys_files,sys_file,boost::is_any_of(","));
    BOOST_FOREACH(const string & my_sys, sys_files) {
        ifstream sys_in(my_sys.c_str());
        if(!sys_in)
            THROW_ERROR(sys_file << " could not be opened for reading");
        if(use_nbest)
            LoadNbests(sys_in, *tgm);
        else
            LoadForests(sys_in, *tgm);
    }

    // Set the weight ranges
    if(config.GetString("weight_ranges") != "") {
        vector<string> ranges, range_vals;
        boost::split(ranges, config.GetString("weight_ranges"), boost::is_any_of(" "));
        BOOST_FOREACH(const string & range, ranges) {
            boost::split(range_vals, range, boost::is_any_of("|"));
            if(range_vals.size() != 2 && range_vals.size() != 3)
                THROW_ERROR("Weight ranges must be in the format MIN|MAX[|NAME]");
            WordId id = (range_vals.size() == 3 ? Dict::WID(range_vals[2]) : -1);
            double min_score = (range_vals[0] == "" ? -DBL_MAX : atoi(range_vals[0].c_str()));
            double max_score = (range_vals[1] == "" ? DBL_MAX  : atoi(range_vals[1].c_str()));
            tgm->SetRange(id, min_score, max_score);
        }
    }

    // Set other tuning options
    tgm->SetGainThreshold(config.GetDouble("threshold"));
    
    // Perform MERT with initial values
    PRINT_DEBUG("Tuning..." << endl, 1); 
    SparseMap best_weights = weights;
    double best_score = tgm->RunTuning(best_weights);
    PRINT_DEBUG("Init: " << Dict::PrintFeatures(best_weights) << " => " << best_score << endl, 1);

    // Perform MERT with random restarts
    for(int i = 1; i < config.GetInt("restarts"); i++) {
        // Print out the current values
        PRINT_DEBUG("Current score: " << best_score << endl, 2);
        PRINT_DEBUG("Current feats: " << Dict::PrintFeatures(best_weights) << endl, 2);
        // Randomize the weights
        SparseMap rand_weights = weights;
        BOOST_FOREACH(SparseMap::value_type & rand_weights, weights)
            rand_weights.second = rand()/(double)RAND_MAX;
        double rand_score = tgm->RunTuning(rand_weights);
        // If the new value is better than the current best, update
        if(rand_score > best_score) {
            best_score = rand_score;
            best_weights = rand_weights;
        }
        PRINT_DEBUG("Rand "<<i<<": " << Dict::PrintFeatures(rand_weights) << " => " << rand_score << endl, 1);
    }

    // Print result
    PRINT_DEBUG("Best: " << Dict::PrintFeatures(best_weights) << " => " << best_score << endl, 0);
    cout << Dict::PrintFeatures(best_weights) << endl;
}
