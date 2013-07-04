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
#include <travatar/eval-measure-ter.h>
#include <travatar/tune-greedy-mert.h>
// #include <travatar/tune-online.h>
#include <travatar/tuning-example-nbest.h>
#include <travatar/tuning-example-forest.h>
#include <travatar/hyper-graph.h>
#include <travatar/tree-io.h>
#include <travatar/output-collector.h>

using namespace travatar;
using namespace std;
using namespace boost;

void BatchTuneRunner::LoadNbests(istream & sys_in, Tune & tgm, istream * stat_in) {
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
        shared_ptr<EvalStats> stats;
        if(stat_in) {
            if(!getline(*stat_in, line))
                THROW_ERROR("Lines in statistic file and system input don't match");
            stats = eval_->ReadStats(line);
        } else {
            stats = eval_->CalculateStats(ref, hyp, id);
        }
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

// Perform tuning
void BatchTuneRunner::DoTuning(const ConfigBatchTune & config) {
    // Chose the tuning method
    shared_ptr<Tune> tgm;
    if(config.GetString("algorithm") == "mert") {
        tgm.reset(new TuneMert);
    } else if(config.GetString("algorithm") == "greedy-mert") {
        tgm.reset(new TuneGreedyMert);
        ((TuneGreedyMert&)*tgm).SetThreads(config.GetInt("threads"));
    }
    // } else if(config.GetString("algorithm") == "online") {
    //     tgm.reset(new TuneOnline);
    // }

    // Open the n-best list if it exists
    bool use_nbest = config.GetString("nbest") != "";
    bool use_forest = config.GetString("forest") != "";
    if(!(use_nbest ^ use_forest))
        THROW_ERROR("Must specify either -nbest or -forest and not both");

    // Open the system files
    string sys_file = use_nbest ? config.GetString("nbest") : config.GetString("forest");
    vector<string> sys_files;
    boost::split(sys_files,sys_file,boost::is_any_of(","));

    // Check if we have stats to read in
    string stat_file = config.GetString("stat_in");
    vector<string> stat_files;
    if(stat_file.length()) {
        if(use_forest)
            THROW_ERROR("Pre-computed statistics files can only be used for n-best tuning");
        boost::split(stat_files,stat_file,boost::is_any_of(","));
        if(stat_files.size() != sys_files.size())
            THROW_ERROR("Number of system outputs and evaluation statistics don't match!");
    }

    // Convert the n-best lists or forests into example pairs for tuning
    PRINT_DEBUG("Loading system output..." << endl, 1);
    for(int i = 0; i < (int)sys_files.size(); i++) {
        // Open the system file
        ifstream sys_in(sys_files[i].c_str());
        if(!sys_in)
            THROW_ERROR(sys_files[i] << " could not be opened for reading");
        // Open the stats file
        shared_ptr<ifstream> stat_in;
        if(stat_files.size() > 0) {
            stat_in.reset(new ifstream(stat_files[i].c_str()));
            if(!*stat_in)
                THROW_ERROR(stat_files[i] << " could not be opened for reading");
        }
        // Actually load the files
        if(use_nbest) LoadNbests(sys_in, *tgm, stat_in.get());
        else          LoadForests(sys_in, *tgm);
    }

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

void BatchTuneRunner::CalculateSentenceStats(const ConfigBatchTune & config, const string & filename) {
    // Open the system file
    ifstream sys_in(config.GetString("nbest").c_str());
    if(!sys_in)
        THROW_ERROR(config.GetString("nbest") << " could not be opened for reading");
    // Open the output file
    ofstream stat_out(filename.c_str());
    if(!stat_out)
        THROW_ERROR(filename << " could not be opened for reading");
    // Process the file one by one
    string line;
    regex threebars(" \\|\\|\\| ");
    while(getline(sys_in, line)) {
        vector<string> columns;
        algorithm::split_regex(columns, line, threebars);
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        int id = atoi(columns[0].c_str());
        Sentence hyp = Dict::ParseWords(columns[1]);
        EvalStatsPtr stats = eval_->CalculateStats(refs_[id], hyp, id);
        stat_out << stats->WriteStats() << endl;
    }
}

// Run the model
void BatchTuneRunner::Run(const ConfigBatchTune & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Open the references
    ifstream ref_in(config.GetMainArg(0).c_str());
    if(!ref_in)
        THROW_ERROR(config.GetMainArg(0) << " could not be opened for reading");

    // Load the references
    PRINT_DEBUG("Loading references..." << endl, 1);
    string line;
    while(getline(ref_in, line)) {
        Sentence ref = Dict::ParseWords(line);
        refs_.push_back(ref);
        ref_len_ += ref.size();
    }

    // Create the evaluation measure
    eval_.reset(EvalMeasure::CreateMeasureFromString(config.GetString("eval")));
    
    // Figure out whether we are tuning or calculating sentence statistics
    string stat_out_filename = config.GetString("stat_out");
    if(stat_out_filename.length()) {
        CalculateSentenceStats(config, stat_out_filename);
    } else {
        DoTuning(config);
    }
    
}
