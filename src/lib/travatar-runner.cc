#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <travatar/thread-pool.h>
#include <travatar/tree-io.h>
#include <travatar/translation-rule.h>
#include <travatar/travatar-runner.h>
#include <travatar/trimmer-nbest.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/lookup-table-marisa.h>
#include <travatar/lookup-table-fsm.h>
#include <travatar/weights.h>
#include <travatar/weights-perceptron.h>
#include <travatar/weights-delayed-perceptron.h>
#include <travatar/lm-composer-bu.h>
#include <travatar/lm-composer-incremental.h>
#include <travatar/binarizer.h>
#include <travatar/eval-measure.h>
#include <travatar/timer.h>
#include <travatar/input-file-stream.h>
#include <travatar/config-travatar-runner.h>
#include <lm/model.hh>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
// #include <boost/algorithm/string.hpp>
#include <fstream>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace lm::ngram;

void TravatarRunnerTask::Run() {
    PRINT_DEBUG("Translating sentence " << sent_ << endl << Dict::PrintWords(tree_graph_->GetWords()) << endl, 1);
    // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*tree_graph_, cerr); cerr << endl; }
    // Binarizer if necessary
    if(runner_->HasBinarizer()) {
        shared_ptr<HyperGraph> bin_graph(runner_->GetBinarizer().TransformGraph(*tree_graph_));
        tree_graph_.swap(bin_graph);
    }
    // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*tree_graph_, cerr); cerr << endl; }
    shared_ptr<HyperGraph> rule_graph(runner_->GetTM().TransformGraph(*tree_graph_));
    rule_graph->ScoreEdges(runner_->GetWeights());
    rule_graph->ResetViterbiScores();

    // If we have an lm, score with the LM
    // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*rule_graph, cerr); cerr << endl; }
    if(runner_->HasLM() && rule_graph->NumNodes() > 0) {
        shared_ptr<HyperGraph> lm_graph(runner_->GetLM().TransformGraph(*rule_graph));
        lm_graph.swap(rule_graph);
    }

    // Calculate the n-best list
    NbestList nbest_list;
    if(rule_graph->NumNodes() > 0) {
        PRINT_DEBUG("SENT " << sent_ << " score: " << rule_graph->GetNode(0)->GetViterbiScore() << endl, 1);
        nbest_list = rule_graph->GetNbest(runner_->GetNbestCount());
    }

    // Print the best answer. This will generally be the answer with the highest score
    // but we could also change it with something like MBR
    int best_answer = 0;
    ostringstream out;
    if((int)nbest_list.size() > best_answer) {
        out << Dict::PrintWords(nbest_list[best_answer]->GetTrgData());
    }
    out << endl;
    collector_->Write(sent_, out.str(), "");

    // If we are printing the n-best list, print it
    if(nbest_collector_ != NULL) {
        ostringstream nbest_out;
        BOOST_FOREACH(const shared_ptr<HyperPath> & path, nbest_list) {
            nbest_out
                << sent_
                << " ||| " << Dict::PrintWords(path->GetTrgData())
                << " ||| " << path->GetScore()
                << " ||| " << Dict::PrintSparseVector(path->CalcFeatures()) << endl;
        }
        nbest_collector_->Write(sent_, nbest_out.str(), "");
    }
    
    // If we are printing a trace, create it
    if(trace_collector_ != NULL && nbest_list.size() > 0) {
        ostringstream trace_out;
        BOOST_FOREACH(const HyperEdge * edge, nbest_list[best_answer]->GetEdges()) {
            trace_out
                << sent_
                << " ||| " << edge->GetHead()->GetSpan()
                << " ||| " << edge->GetSrcStr() 
                << " ||| " << Dict::PrintAnnotatedVector(edge->GetTrgData())
                << " ||| " << Dict::PrintSparseVector(edge->GetFeatures())
                << endl;
        }
        trace_collector_->Write(sent_, trace_out.str(), "");
    }

    // If we are printing a forest, print it
    if(forest_collector_ != NULL) {
        // Trim if needed
        shared_ptr<HyperGraph> out_for = rule_graph;
        if(runner_->HasTrimmer())
            out_for.reset(runner_->GetTrimmer().TransformGraph(*out_for));
        // Print
        ostringstream forest_out;
        JSONTreeIO io;
        io.WriteTree(*out_for, forest_out);
        forest_out << endl;
        forest_collector_->Write(sent_, forest_out.str(), "");
    }

    // If we are tuning load the next references and check the weights
    if(runner_->GetDoTuning())
        runner_->GetWeights().Adjust(tree_graph_->GetWords(), refs_, runner_->GetEvalMeasure(), nbest_list);
}


// Run the model
void TravatarRunner::Run(const ConfigTravatarRunner & config) {

    // Load all the variables
    GlobalVars::debug = config.GetInt("debug");
    GlobalVars::trg_factors = config.GetInt("trg_factors");
    nbest_count_ = config.GetInt("nbest");
    threads_ = config.GetInt("threads");

    // Create the timer
    Timer timer;
    timer.start();

    // Set weights
    if(config.GetString("weight_vals") == "")
        THROW_ERROR("You must specify weights through -weight_vals. If you really don't want any weights, just set -weight_vals dummy=0");
    SparseMap init_weights = Dict::ParseSparseMap(config.GetString("weight_vals"));

    // Create the appropriate weights
    // If we are using online tuning, choose weights according to the tuning method,
    // otherwise choose plain weights
    do_tuning_ = true;
    if(config.GetString("tune_update") == "perceptron") {
        WeightsPerceptron * ptr = new WeightsPerceptron(init_weights);
        ptr->SetL1Coeff(config.GetDouble("tune_l1_coeff"));
        weights_.reset(ptr);
    } else if(config.GetString("tune_update") == "delayed") {
        weights_.reset(new WeightsDelayedPerceptron(init_weights));
    } else if(config.GetString("tune_update") == "none") {
        weights_.reset(new Weights(init_weights));
        do_tuning_ = false;
    } else {
        THROW_ERROR("Invalid value for tune_update: "<<config.GetString("tune_update"));
    }    
    vector<shared_ptr<istream> > tune_ins;
    // If we need to do tuning
    if(do_tuning_) {
        if(threads_ > 1)
            THROW_ERROR("Online tuning and threads cannot be combined at the moment");
        // Check that a place to write the weights has been specified
        string weight_out_file = config.GetString("tune_weight_out");
        if(weight_out_file.length() == 0)
            THROW_ERROR("Tuning is active, but -tune_weight_out was not specified");
        ofstream weight_out(weight_out_file.c_str());
        if(!weight_out)
            THROW_ERROR("Could open tune_weight_out file: " << config.GetString("tune_weight_out"));
        // Set the evaluation measure to be used
        tune_eval_measure_.reset(EvalMeasure::CreateMeasureFromString(config.GetString("tune_loss")));
        // And open the reference files
        vector<string> ref_files = config.GetStringArray("tune_ref_files");
        if(ref_files.size() == 0)
            THROW_ERROR("When tuning, must specify at least one reference in tune_ref_files");
        BOOST_FOREACH(const string & file, ref_files)
            tune_ins.push_back(shared_ptr<istream>(new ifstream(file.c_str())));
        // Set the weight ranges
        if(config.GetString("tune_weight_ranges") != "") {
            vector<string> ranges = Tokenize(config.GetString("tune_weight_ranges"), ' '), range_vals;
            BOOST_FOREACH(const string & range, ranges) {
                range_vals = Tokenize(range, '|');
                if(range_vals.size() != 2 && range_vals.size() != 3)
                    THROW_ERROR("Weight ranges must be in the format MIN|MAX[|NAME]");
                WordId id = (range_vals.size() == 3 ? Dict::WID(range_vals[2]) : -1);
                double min_score = (range_vals[0] == "" ? -DBL_MAX : atoi(range_vals[0].c_str()));
                double max_score = (range_vals[1] == "" ? DBL_MAX  : atoi(range_vals[1].c_str()));
                weights_->SetRange(id, min_score, max_score);
            }
        }
    }

    // Create the binarizer_
    binarizer_.reset(Binarizer::CreateBinarizerFromString(config.GetString("binarize")));

    // Get the input format parser
    shared_ptr<TreeIO> tree_io;
    if(config.GetString("in_format") == "penn")
        tree_io = shared_ptr<TreeIO>(new PennTreeIO);
    else if(config.GetString("in_format") == "egret")
        tree_io = shared_ptr<TreeIO>(new EgretTreeIO);
    else if(config.GetString("in_format") == "moses")
        tree_io = shared_ptr<TreeIO>(new MosesXMLTreeIO);
    else if(config.GetString("in_format") == "word") 
        tree_io = shared_ptr<TreeIO>(new WordTreeIO);
    else
        THROW_ERROR("Bad in_format option " << config.GetString("in_format"));

    // Load the language model
    PRINT_DEBUG("Loading language model [" << timer << " sec]" << endl, 1);
    if(config.GetString("lm_file") != "") {
        // Set the LM Composer
        if(config.GetString("search") == "cp") {
            LMComposerBU * bu = new LMComposerBU(config.GetString("lm_file"));
            bu->SetStackPopLimit(config.GetInt("pop_limit"));
            bu->SetChartLimit(config.GetInt("chart_limit"));
            bu->UpdateWeights(weights_->GetCurrent());
            lm_.reset(bu);
        } else if(config.GetString("search") == "inc") {
            LMComposerIncremental * inc = new LMComposerIncremental(config.GetString("lm_file"));
            inc->SetStackPopLimit(config.GetInt("pop_limit"));
            inc->UpdateWeights(weights_->GetCurrent());
            lm_.reset(inc);
        } else {
            THROW_ERROR("Unknown search algorithm: " << config.GetString("search"));
        }
    }

    // Load the rule table
    PRINT_DEBUG(endl << "Loading translation model [" << timer << " sec]" << endl, 1);
    vector<string> tm_files = config.GetStringArray("tm_file");
    if(config.GetString("tm_storage") == "hash") {
        LookupTableHash * hash_tm_ = LookupTableHash::ReadFromFile(tm_files[0]);
        hash_tm_->SetMatchAllUnk(config.GetBool("all_unk"));
        tm_.reset(hash_tm_);
    } else if(config.GetString("tm_storage") == "marisa") {
        LookupTableMarisa * marisa_tm_ = LookupTableMarisa::ReadFromFile(tm_files[0]);
        marisa_tm_->SetMatchAllUnk(config.GetBool("all_unk"));
        tm_.reset(marisa_tm_);
    }  else if (config.GetString("tm_storage") == "fsm") {
        LookupTableFSM * fsm_tm_ = LookupTableFSM::ReadFromFiles(tm_files);
        fsm_tm_->SetDeleteUnknown(config.GetBool("delete_unknown"));
        fsm_tm_->SetRootSymbol(Dict::WID(config.GetString("root_symbol")));
        fsm_tm_->SetDefaultSymbol(Dict::WID(config.GetString("default_symbol")));
        fsm_tm_->SetSpanLimits(config.GetIntArray("hiero_span_limit"));
        tm_.reset(fsm_tm_);
    } else {
        THROW_ERROR("Unknown storage type: " << config.GetString("tm_storage"));
    }

    // Open the n-best output stream if it exists
    scoped_ptr<ostream> nbest_out;
    scoped_ptr<OutputCollector> nbest_collector;
    if(config.GetString("nbest_out") != "") {
        nbest_out.reset(new ofstream(config.GetString("nbest_out").c_str()));
        if(!*nbest_out)
            THROW_ERROR("Could not open nbest output file: " << config.GetString("nbest_out"));
        nbest_collector.reset(new OutputCollector(nbest_out.get(), &cerr, config.GetBool("buffer")));
    } else if (!do_tuning_) {
        nbest_count_ = 1;
    }

    // Open the forest output stream if it exists
    scoped_ptr<ostream> forest_out;
    scoped_ptr<OutputCollector> forest_collector;
    if(config.GetString("forest_out") != "") {
        forest_out.reset(new ofstream(config.GetString("forest_out").c_str()));
        if(!*forest_out)
            THROW_ERROR("Could not open forest output file: " << config.GetString("forest_out"));
        forest_collector.reset(new OutputCollector(forest_out.get(), &cerr, config.GetBool("buffer")));
    } 

    // Get the class to trim the forest if necessary
    scoped_ptr<Trimmer> trimmer_;
    if(config.GetInt("forest_nbest_trim") != 0)
        trimmer_.reset(new TrimmerNbest(config.GetInt("forest_nbest_trim")));

    // Open the trace output stream if it exists
    scoped_ptr<ostream> trace_out;
    scoped_ptr<OutputCollector> trace_collector;
    if(config.GetString("trace_out") != "") {
        trace_out.reset(new ofstream(config.GetString("trace_out").c_str()));
        if(!*trace_out)
            THROW_ERROR("Could not open trace output file: " << config.GetString("trace_out"));
        trace_collector.reset(new OutputCollector(trace_out.get(), &cerr, config.GetBool("buffer")));
    }

    // Create the thread pool
    ThreadPool pool(threads_, threads_*5);
    OutputCollector collector;
    // Process one at a time
    int sent = 0;
    string line;
    PRINT_DEBUG("Started translating [" << timer << " sec]" << endl, 1);
    while(1) {
        // Load the tree
        shared_ptr<HyperGraph> tree_graph(tree_io->ReadTree(std::cin));
        if(tree_graph.get() == NULL) break;

        // If we are tuning load the next references and check the weights
        vector<Sentence> refs;
        if(do_tuning_) {
            BOOST_FOREACH(const shared_ptr<istream> & in, tune_ins) {
                string line;
                if(!getline(*in, line)) THROW_ERROR("Reference file is too short");
                refs.push_back(Dict::ParseWords(line));
            }
        }

        TravatarRunnerTask *task = new TravatarRunnerTask(sent++, tree_graph, this, refs, &collector, nbest_collector.get(), trace_collector.get(), forest_collector.get());
        if(threads_ == 1) {
            task->Run();
            delete task;
        } else {
            pool.Submit(task);
        }
        cerr << (sent%100==0?'!':'.'); cerr.flush();
    }
    pool.Stop(true);
    PRINT_DEBUG(endl << "Done translating [" << timer << " sec]" << endl, 1);
    
    if(do_tuning_) {
        // Load the features from the weight file
        ofstream weight_out(config.GetString("tune_weight_out").c_str());
        cerr << "Writing weight file to "<<config.GetString("tune_weight_out")<<"..." << endl;
        if(!weight_out)
            THROW_ERROR("Could open tune_weight_out file: " << config.GetString("tune_weight_out"));
        BOOST_FOREACH(const SparseMap::value_type & val, weights_->GetFinal())
            weight_out << Dict::WSym(val.first) << "=" << val.second << endl;
        weight_out.close();
    }

}
