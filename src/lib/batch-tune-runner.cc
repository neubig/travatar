#include <travatar/batch-tune-runner.h>

#include <travatar/eval-measure-loader.h>
#include <travatar/config-batch-tune.h>
#include <travatar/input-file-stream.h>
#include <travatar/tune-mert.h>
#include <travatar/tune-greedy-mert.h>
#include <travatar/tune-lbfgs.h>
#include <travatar/tune-online.h>
#include <travatar/tuning-example-nbest.h>
#include <travatar/tuning-example-forest.h>
#include <travatar/gradient-xeval.h>
#include <travatar/hyper-graph.h>
#include <travatar/tree-io.h>
#include <travatar/output-collector.h>
#include <travatar/thread-pool.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace travatar;
using namespace std;
using namespace boost;


BatchTuneRunnerTask::BatchTuneRunnerTask(
        int task_id, const std::string & task_name,
        Tune & tune, const SparseMap & weights) :
    task_id_(task_id), task_name_(task_name), tune_(&tune),
    weights_(weights), score_(-REAL_MAX) { }

void BatchTuneRunnerTask::Run() {
    score_ = tune_->RunTuning(weights_);
    PRINT_DEBUG(task_name_<<": " << Dict::PrintSparseMap(weights_) << " => " << score_ << endl, 1);
}

void BatchTuneRunner::LoadNbests(istream & sys_in, Tune & tune, istream * stat_in) {
    string line;
    while(getline(sys_in, line)) {
        vector<string> columns = Tokenize(line, " ||| ");
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        // Get the number
        int id = atoi(columns[0].c_str());
        // Get the factors
        vector<Sentence> hyps = Dict::ParseWordVector(columns[1]);
        // Get the features
        SparseVector feat = Dict::ParseSparseVector(columns[3]);
        // Calculate the score
        const vector<Sentence> & ref = refs_[id];
        boost::shared_ptr<EvalStats> stats;
        if(stat_in) {
            if(!getline(*stat_in, line))
                THROW_ERROR("Lines in statistic file and system input don't match");
            stats = eval_->ReadStats(line);
        } else {
            stats = eval_->CalculateCachedStats(ref, hyps, id);
        }
        // Add the example
        while((int)tune.NumExamples() <= id) {
            if(id % 100 == 0)
                PRINT_DEBUG(id << ".", 1);
            tune.AddExample(boost::shared_ptr<TuningExample>(new TuningExampleNbest()));
        }
        ((TuningExampleNbest&)tune.GetExample(id)).AddHypothesis(feat, stats);
    }
    PRINT_DEBUG(endl, 1);
}

void BatchTuneRunner::LoadForests(istream & sys_in, Tune & tune) {
    JSONTreeIO io;
    HyperGraph * curr_ptr;
    int id = 0;
    bool normalize_len = false;
    while((curr_ptr = io.ReadTree(sys_in)) != NULL) {
        if(id % 100 == 0)
            PRINT_DEBUG(id << ".", 1);
        PRINT_DEBUG("Loading line " << id << endl, 1);
        const std::vector<Sentence> & ref = refs_[id];
        // Add the example
        if((int)tune.NumExamples() <= id) {
            Real norm = (normalize_len ? ref.size() / (Real)ref_len_ : 1.0 / refs_.size());
            tune.AddExample(
                boost::shared_ptr<TuningExample>(
                    new TuningExampleForest(
                        eval_.get(),
                        ref, id, norm)));
        }
        ((TuningExampleForest&)tune.GetExample(id)).AddHypothesis(boost::shared_ptr<HyperGraph>(curr_ptr));
        id++;
    }
}

// Perform tuning
void BatchTuneRunner::DoTuning(const ConfigBatchTune & config) {
    
    // Save number of threads and runs
    int threads = config.GetInt("threads");
    int runs = config.GetInt("restarts")+2;
    
    // Chose the tuning method
    boost::shared_ptr<Tune> tune;
    if(config.GetString("algorithm") == "mert") {
        TuneMert *tm = new TuneMert;
        tm->SetDirections(config.GetString("mert_directions"));
        tune.reset(tm);
    } else if(config.GetString("algorithm") == "greedy-mert") {
        TuneGreedyMert *tgm = new TuneGreedyMert;
        tgm->SetThreads(threads);
        tune.reset(tgm);
        threads = 1; // Threading is done inside greedy mert
    } else if(config.GetString("algorithm") == "lbfgs") {
        GradientXeval * gx = new GradientXeval;
        gx->SetL2Coefficient(config.GetReal("l2"));
        gx->SetEntCoefficient(config.GetReal("ent"));
        TuneLbfgs * tl = new TuneLbfgs(gx);
        tl->SetL1Coefficient(config.GetReal("l1"));
        tune.reset(tl);
    } else if(config.GetString("algorithm") == "online" || config.GetString("algorithm") == "onlinepro") {
        TuneOnline * online = new TuneOnline;
        online->SetUpdate(config.GetString("update"));
        online->SetLearningRate(config.GetReal("rate"));
        online->SetMarginScale(config.GetReal("margin_scale"));
        if(config.GetString("algorithm") == "onlinepro")
            online->SetAlgorithm("pro");
        tune.reset(online);
    } else {
        THROW_ERROR("Unknown tuning algorithm " << config.GetString("algorithm"));
    }

    // Load the features from the weight file
    SparseMap weights;
    if(config.GetString("weight_in") != "") {
        PRINT_DEBUG("Reading weight file from "<<config.GetString("weight_in")<<"..." << endl, 1);
        ifstream weight_in(config.GetString("weight_in").c_str());
        if(!weight_in)
            THROW_ERROR("Could not find weights: " << config.GetString("weight_in"));
        weights = Dict::ParseSparseMap(weight_in);
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
            Real min_score = (range_vals[0] == "" ? -REAL_MAX : atoi(range_vals[0].c_str()));
            Real max_score = (range_vals[1] == "" ? REAL_MAX  : atoi(range_vals[1].c_str()));
            tune->SetRange(id, min_score, max_score);
        }
    }

    // Set other tuning options
    tune->SetGainThreshold(config.GetReal("threshold"));

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
        InputFileStream sys_in(sys_files[i].c_str());
        if(!sys_in)
            THROW_ERROR(sys_files[i] << " could not be opened for reading");
        // Open the stats file
        boost::shared_ptr<ifstream> stat_in;
        if(stat_files.size() > 0) {
            stat_in.reset(new ifstream(stat_files[i].c_str()));
            if(!*stat_in)
                THROW_ERROR(stat_files[i] << " could not be opened for reading");
        }
        // Actually load the files
        if(use_nbest) LoadNbests(sys_in, *tune, stat_in.get());
        else          LoadForests(sys_in, *tune);
    }

    // If there is any shared initialization to be done, do it here
    tune->Init(weights);

    // Build the thread pool
    ThreadPool pool(threads, threads*5);
    pool.SetDeleteTasks(false);
    
    // Set up tasks for each amount of weights
    vector<boost::shared_ptr<BatchTuneRunnerTask> > tasks(runs);
    tasks[0] = boost::shared_ptr<BatchTuneRunnerTask>(new BatchTuneRunnerTask(0, "Init", *tune, weights));
    pool.Submit(tasks[0].get());

    // Set up zeroed weights
    tasks[1] = boost::shared_ptr<BatchTuneRunnerTask>(new BatchTuneRunnerTask(1, "Zero", *tune, SparseMap()));
    pool.Submit(tasks[1].get());
    
    // Randomize if necessary
    if(config.GetInt("rand_seed") == 0)
        srand(time(NULL));
    else
        srand(config.GetInt("rand_seed"));
    for(int i = 2; i < runs; i++) {
        // Randomize the weights
        SparseMap rand_weights = weights;
        BOOST_FOREACH(SparseMap::value_type & rw, rand_weights)
            if(rw.first != tune->GetScaleId())
                rw.second *= rand()/(Real)RAND_MAX;
        ostringstream oss; oss << "Rand " << i;
        tasks[i] = boost::shared_ptr<BatchTuneRunnerTask>(new BatchTuneRunnerTask(i, oss.str(), *tune, rand_weights));
        pool.Submit(tasks[i].get());
    }
    pool.Stop(true);

    // Find the best result
    SparseMap best_weights = tasks[0]->GetWeights();
    Real best_score = tasks[0]->GetScore();
    for(int i = 1; i < runs; i++) {
        // If the new value is better than the current best, update
        if(tasks[i]->GetScore() > best_score) {
            best_score = tasks[i]->GetScore();
            best_weights = tasks[i]->GetWeights();
        }
    }

    // Print result
    PRINT_DEBUG("Best: " << Dict::PrintSparseMap(best_weights) << " => " << best_score << endl, 0);
    cout << Dict::PrintSparseMap(best_weights) << endl;
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
    while(getline(sys_in, line)) {
        vector<string> columns = Tokenize(line, " ||| ");
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        int id = atoi(columns[0].c_str());
        vector<Sentence> hyps = Dict::ParseWordVector(columns[1]);
        EvalStatsPtr stats = eval_->CalculateCachedStats(refs_[id], hyps, id);
        stat_out << stats->WriteStats() << endl;
    }
}

// Run the model
void BatchTuneRunner::Run(const ConfigBatchTune & config) {

    // Set the debugging level and number of factors
    GlobalVars::debug = config.GetInt("debug");
    GlobalVars::trg_factors = config.GetInt("trg_factors");

    // Open the references
    ifstream ref_in(config.GetMainArg(0).c_str());
    if(!ref_in)
        THROW_ERROR(config.GetMainArg(0) << " could not be opened for reading");

    // Load the references
    PRINT_DEBUG("Loading references..." << endl, 1);
    string line;
    while(getline(ref_in, line)) {
        std::vector<Sentence> ref = Dict::ParseWordVector(line);
        refs_.push_back(ref);
        ref_len_ += ref.size();
    }

    // Create the evaluation measure
    eval_.reset(EvalMeasureLoader::CreateMeasureFromString(config.GetString("eval")));
    
    // Figure out whether we are tuning or calculating sentence statistics
    string stat_out_filename = config.GetString("stat_out");
    if(stat_out_filename.length()) {
        CalculateSentenceStats(config, stat_out_filename);
    } else {
        DoTuning(config);
    }
    
}
