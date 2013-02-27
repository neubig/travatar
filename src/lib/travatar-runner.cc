#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <travatar/tree-io.h>
#include <travatar/translation-rule.h>
#include <travatar/travatar-runner.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/lookup-table-marisa.h>
#include <travatar/weights.h>
#include <travatar/weights-perceptron.h>
#include <travatar/weights-delayed-perceptron.h>
#include <travatar/lm-composer-bu.h>
#include <travatar/binarizer-directional.h>
#include <travatar/binarizer-cky.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/eval-measure.h>
#include <travatar/config-travatar-runner.h>
#include <lm/model.hh>
#include <fstream>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace lm::ngram;

// Run the model
void TravatarRunner::Run(const ConfigTravatarRunner & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Set weights
    if(config.GetString("weight_vals") == "")
        THROW_ERROR("You must specify weights through -weight_vals. If you really don't want any weights, just set -weight_vals dummy=0");
    SparseMap init_weights = Dict::ParseFeatures(config.GetString("weight_vals"));

    // Create the appropriate weights
    // If we are using online tuning, choose weights according to the tuning method,
    // otherwise choose plain weights
    shared_ptr<Weights> weights;
    bool do_tuning = true;
    if(config.GetString("tune_update") == "perceptron") {
        WeightsPerceptron * ptr = new WeightsPerceptron(init_weights);
        ptr->SetL1Coeff(config.GetDouble("tune_l1_coeff"));
        weights.reset(ptr);
    } else if(config.GetString("tune_update") == "delayed") {
        weights.reset(new WeightsDelayedPerceptron(init_weights));
    } else if(config.GetString("tune_update") == "none") {
        weights.reset(new Weights(init_weights));
        do_tuning = false;
    } else {
        THROW_ERROR("Invalid value for tune_update: "<<config.GetString("tune_update"));
    }    
    vector<shared_ptr<istream> > tune_ins;
    shared_ptr<EvalMeasure> tune_eval_measure;
    // If we need to do tuning
    if(do_tuning) {
        // Check that a place to write the weights has been specified
        string weight_out_file = config.GetString("tune_weight_out");
        if(weight_out_file.length() == 0)
            THROW_ERROR("Tuning is active, but -tune_weight_out was not specified");
        ofstream weight_out(weight_out_file.c_str());
        if(!weight_out)
            THROW_ERROR("Could open tune_weight_out file: " << config.GetString("tune_weight_out"));
        // Set the evaluation measure to be used
        if(config.GetString("tune_loss") == "bleu") {
            EvalMeasureBleu * meas = new EvalMeasureBleu;
            meas->SetSmoothVal(1.0); // Use BLEU+1
            tune_eval_measure.reset(meas);
        } else if(config.GetString("tune_loss") == "ribes") {
            EvalMeasureRibes * meas = new EvalMeasureRibes;
            tune_eval_measure.reset(meas);
        } else {
            THROW_ERROR("Invalid value for tune_loss: "<<config.GetString("tune_loss"));
        }
        // And open the reference files
        vector<string> ref_files = config.GetStringArray("tune_ref_files");
        if(ref_files.size() == 0)
            THROW_ERROR("When tuning, must specify at least one reference in tune_ref_files");
        BOOST_FOREACH(const string & file, ref_files)
            tune_ins.push_back(shared_ptr<istream>(new ifstream(file.c_str())));
        // Set the weight ranges
        if(config.GetString("tune_weight_ranges") != "") {
            vector<string> ranges, range_vals;
            boost::split(ranges, config.GetString("tune_weight_ranges"), boost::is_any_of(" "));
            BOOST_FOREACH(const string & range, ranges) {
                boost::split(range_vals, range, boost::is_any_of("|"));
                if(range_vals.size() != 2 && range_vals.size() != 3)
                    THROW_ERROR("Weight ranges must be in the format MIN|MAX[|NAME]");
                WordId id = (range_vals.size() == 3 ? Dict::WID(range_vals[2]) : -1);
                double min_score = (range_vals[0] == "" ? -DBL_MAX : atoi(range_vals[0].c_str()));
                double max_score = (range_vals[1] == "" ? DBL_MAX  : atoi(range_vals[1].c_str()));
                weights->SetRange(id, min_score, max_score);
            }
        }
    }

    // Create the binarizer
    shared_ptr<GraphTransformer> binarizer;
    if(config.GetString("binarize") == "left") {
        binarizer.reset(new BinarizerDirectional(BinarizerDirectional::BINARIZE_LEFT));
    } else if(config.GetString("binarize") == "right") {
        binarizer.reset(new BinarizerDirectional(BinarizerDirectional::BINARIZE_RIGHT));
    } else if(config.GetString("binarize") == "cky") {
        binarizer.reset(new BinarizerCKY);
    } else if(config.GetString("binarize") != "none") {
        THROW_ERROR("Invalid binarizer type " << config.GetString("binarizer"));
    }

    // Get the input format parser
    TreeIO * tree_io;
    if(config.GetString("in_format") == "penn")
        tree_io = new PennTreeIO;
    else if(config.GetString("in_format") == "egret")
        tree_io = new EgretTreeIO;
    else
        THROW_ERROR("Bad in_format option " << config.GetString("in_format"));

    // Load the language model
    shared_ptr<LMComposerBU> lm;
    if(config.GetString("lm_file") != "") {
        LMComposerBU * bu = 
            new LMComposerBU(new Model(config.GetString("lm_file").c_str()));
        bu->SetLMWeight(weights->GetCurrent(Dict::WID("lm")));
        bu->SetStackPopLimit(config.GetInt("pop_limit"));
        lm.reset(bu);
    }

    // Load the rule table
    ifstream tm_in(config.GetString("tm_file").c_str());
    cerr << "Reading TM file from "<<config.GetString("tm_file")<<"..." << endl;
    if(!tm_in)
        THROW_ERROR("Could not find TM: " << config.GetString("tm_file"));

    // Load the translation model
    shared_ptr<LookupTable> tm;
    if(config.GetString("tm_storage") == "hash")
        tm.reset(LookupTableHash::ReadFromRuleTable(tm_in));
    else if(config.GetString("tm_storage") == "marisa")
        tm.reset(LookupTableMarisa::ReadFromRuleTable(tm_in));
    tm->SetMatchAllUnk(config.GetBool("all_unk"));
    tm_in.close();

    // Open the n-best output stream if it exists
    int nbest_count = config.GetInt("nbest");
    scoped_ptr<ostream> nbest_out;
    if(config.GetString("nbest_out") != "") {
        nbest_out.reset(new ofstream(config.GetString("nbest_out").c_str()));
        if(!*nbest_out)
            THROW_ERROR("Could not open nbest output file: " << config.GetString("nbest_out"));
    } else if (!do_tuning) {
        nbest_count = 1;
    }

    // Open the forest output stream if it exists
    scoped_ptr<ostream> forest_out;
    if(config.GetString("forest_out") != "") {
        forest_out.reset(new ofstream(config.GetString("forest_out").c_str()));
        if(!*forest_out)
            THROW_ERROR("Could not open forest output file: " << config.GetString("forest_out"));
    } 

    // Open the trace output stream if it exists
    scoped_ptr<ostream> trace_out;
    if(config.GetString("trace_out") != "") {
        trace_out.reset(new ofstream(config.GetString("trace_out").c_str()));
        if(!*trace_out)
            THROW_ERROR("Could not open trace output file: " << config.GetString("trace_out"));
    }

    // Process one at a time
    int sent = 0;
    string line;
    cerr << "Translating..." << endl;
    while(1) {
        shared_ptr<HyperGraph> tree_graph(tree_io->ReadTree(std::cin));
        if(tree_graph.get() == NULL) break;
        PRINT_DEBUG("Printing sentence " << sent << endl 
                    << Dict::PrintWords(tree_graph->GetWords()) << endl, 1);
        // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*tree_graph, cerr); cerr << endl; }
        // Binarizer if necessary
        if(binarizer.get() != NULL) {
            shared_ptr<HyperGraph> bin_graph(binarizer->TransformGraph(*tree_graph));
            tree_graph.swap(bin_graph);
        }
        // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*tree_graph, cerr); cerr << endl; }
        shared_ptr<HyperGraph> rule_graph(tm->TransformGraph(*tree_graph));
        rule_graph->ScoreEdges(*weights);
        rule_graph->ResetViterbiScores();

        // If we have an lm, score with the LM
        // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*rule_graph, cerr); cerr << endl; }
        if(lm.get() != NULL) {
            shared_ptr<HyperGraph> lm_graph(lm->TransformGraph(*rule_graph));
            lm_graph.swap(rule_graph);
        }

        // Calculate the n-best list
        NbestList nbest_list = rule_graph->GetNbest(nbest_count, tree_graph->GetWords());

        // Print the best answer. This will generally be the answer with the highest score
        // but we could also change it with something like MBR
        int best_answer = 0;
        cout << Dict::PrintWords(nbest_list[best_answer]->GetWords()) << endl;

        // If we are printing the n-best list, print it
        if(nbest_out.get() != NULL) {
            BOOST_FOREACH(const shared_ptr<HyperPath> & path, nbest_list) {
                *nbest_out
                    << sent
                    << " ||| " << Dict::PrintWords(path->GetWords())
                    << " ||| " << path->GetScore()
                    << " ||| " << Dict::PrintFeatures(path->CalcFeatures()) << endl;
            }
        }
        
        // If we are printing a trace, create it
        if(trace_out.get() != NULL) {
            BOOST_FOREACH(const HyperEdge * edge, nbest_list[0]->GetEdges()) {
                *trace_out
                    << sent
                    << " ||| " << edge->GetHead()->GetSpan()
                    << " ||| " << edge->GetRuleStr() 
                    << " ||| " << Dict::PrintAnnotatedWords(edge->GetTrgWords())
                    << " ||| " << Dict::PrintFeatures(edge->GetFeatures())
                    << endl;
            }
        }

        // If we are printing a forest, print it
        if(forest_out.get() != NULL) {
            JSONTreeIO io;
            io.WriteTree(*rule_graph, *forest_out);
            *forest_out << endl;
        }

        // If we are tuning load the next references and check the weights
        if(do_tuning) {
            vector<Sentence> refs;
            BOOST_FOREACH(const shared_ptr<istream> & in, tune_ins) {
                string line;
                if(!getline(*in, line)) THROW_ERROR("Reference file is too short");
                refs.push_back(Dict::ParseWords(line));
            }
            cerr << "TUNING" << endl;
            weights->Adjust(*tune_eval_measure, refs, nbest_list);
            cerr << "DONE TUNING" << endl;
        }

        sent++;
        cerr << (sent%100==0?'!':'.'); cerr.flush();
    }
    
    if(do_tuning) {
        // Load the features from the weight file
        ofstream weight_out(config.GetString("tune_weight_out").c_str());
        cerr << "Writing weight file to "<<config.GetString("tune_weight_out")<<"..." << endl;
        if(!weight_out)
            THROW_ERROR("Could open tune_weight_out file: " << config.GetString("tune_weight_out"));
        BOOST_FOREACH(const SparseMap::value_type & val, weights->GetFinal())
            weight_out << Dict::WSym(val.first) << "=" << val.second << endl;
        weight_out.close();
    }

    cerr << endl << "Done translating..." << endl;

}
