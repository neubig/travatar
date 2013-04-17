#include <iostream>
#include <fstream>
#include <algorithm>
#include <travatar/util.h>
#include <travatar/mt-evaluator-runner.h>
#include <travatar/config-mt-evaluator-runner.h>
#include <travatar/dict.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/eval-measure-ter.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void MTEvaluatorRunner::Run(const ConfigMTEvaluatorRunner & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Load the reference
    ifstream refin(config.GetString("ref").c_str());
    if(!refin) THROW_ERROR("Could not open reference: " << config.GetString("ref"));
    vector< vector<Sentence> > ref_sentences(1);
    string line;
    while(getline(refin, line))
        ref_sentences[0].push_back(Dict::ParseWords(line));
    int ref_len = ref_sentences[0].size();
    
    // Load the evaluation measure
    vector<shared_ptr<EvalMeasure> > eval_measures;
    vector<string> eval_ids;
    algorithm::split(eval_ids, config.GetString("eval"), is_any_of(" "));
    BOOST_FOREACH(const string & eval, eval_ids)
        eval_measures.push_back(shared_ptr<EvalMeasure>(EvalMeasure::CreateMeasureFromString(eval)));
    int eval_count = eval_measures.size();

    // If we are doing bootstrap resampling to calculate statistical significance, create random sets
    int bootstrap = config.GetInt("bootstrap");
    vector<vector<int> > bootstrap_sets;
    if(bootstrap) {
        bootstrap_sets.resize(ref_len);
        vector<int> ids(ref_len);
        for(int i = 0; i < ref_len; i++) ids[i] = i;
        for(int i = 0; i < bootstrap; i++) {
            random_shuffle(ids.begin(), ids.end());
            PRINT_DEBUG("Set " << i << ":", 2);
            for(int j = 0; j < ref_len/2; j++) {
                bootstrap_sets[ids[j]].push_back(i);
                PRINT_DEBUG(" " << ids[j], 2);
            }
            PRINT_DEBUG(endl, 2);
        }
    }

    // Vectors to hold the bootstrap stats
    vector<string> bootstrap_files;
    vector<double> bootstrap_scores;
    // Calculate the scores
    BOOST_FOREACH(const string & filename, config.GetMainArgs()) {
        // Set up the total stats for each measure
        vector<EvalStatsPtr> total_stats(eval_count);
        // Setup the bootstrap stats
        vector<vector<EvalStatsPtr> > bootstrap_stats(eval_count);
        BOOST_FOREACH(vector<EvalStatsPtr> & bs, bootstrap_stats) bs.resize(bootstrap);
        // Do the processing
        int id = 0;
        ifstream sysin(filename.c_str());
        if(!sysin) THROW_ERROR("Could not open system file: " << filename);
        while(getline(sysin, line)) {
            Sentence sys_sent = Dict::ParseWords(line);
            for(int i = 0; i < eval_count; i++) {
                EvalStatsPtr stats = eval_measures[i]->CalculateStats(ref_sentences[0][id],sys_sent);
                // Add the regular stats
                if(total_stats[i].get() == NULL) total_stats[i] = stats->Clone();
                else                             total_stats[i]->PlusEquals(*stats);
                // Add to each bootstrap set that this sentence is a part of
                if(bootstrap) {
                    BOOST_FOREACH(int j, bootstrap_sets[id]) {
                        if(bootstrap_stats[i][j].get() == NULL) bootstrap_stats[i][j] = stats->Clone();
                        else                                    bootstrap_stats[i][j]->PlusEquals(*stats);
                        PRINT_DEBUG("bootstrap_stats[" << i << "][" << j << "] == " << bootstrap_stats[i][j]->ConvertToString() << endl, 3);
                    }
                }
            }
            id++;
        }
        int col = 0;
        // Print the evaluation for this file, with the filename if multiple files are being evaluated
        if(config.GetMainArgs().size() > 1) { cout << filename; col++; }
        if(id == ref_len) {
            // Add this to the names of the files in the bootstrap matrix
            if(bootstrap) bootstrap_files.push_back(filename);
            // Print the stats
            for(int i = 0; i < (int)total_stats.size(); i++) {
                if(col++) cout << "\t";
                cout << total_stats[i]->ConvertToString();
                // Add it to the bootstrap matrix and calculate all scores
                if(bootstrap) {
                    shared_ptr<vector<double> > my_vec(new vector<double>(bootstrap));
                    for(int j = 0; j < bootstrap; j++) {
                        PRINT_DEBUG(endl << "bootstrap_stats[" << i << "][" << j << "] == " << bootstrap_stats[i][j]->ConvertToString() << " @ " << bootstrap_scores.size() << endl, 3);
                        bootstrap_scores.push_back(bootstrap_stats[i][j]->ConvertToScore());
                    }
                }
            }
        } else {
            if(col++) cout << "\t";
            cout << "FAILED: Reference (" << ref_len << ") and System (" << id << ") lengths don't match";
        }
        cout << endl;
    }
    
    // Print the bootstrapping results
    if(bootstrap) {
        cout << endl << "Bootstrap Resampling Significance:" << endl;
        int bootstrap_len = bootstrap_files.size();
        for(int i = 0; i < bootstrap_len-1; i++) {
            cout << bootstrap_files[i] << endl;
            for(int j = i+1; j < bootstrap_len; j++) {
                cout << " " << bootstrap_files[j] << endl;
                int idi = i*eval_count*bootstrap, idj = j*eval_count*bootstrap;
                for(int k = 0; k < eval_count; k++) {
                    int win = 0, tie = 0, loss = 0;
                    for(int l = 0; l < bootstrap; l++, idi++, idj++) {
                        PRINT_DEBUG("   "<<bootstrap_scores[idi]<<" vs. "<<bootstrap_scores[idj], 2);
                        if(bootstrap_scores[idi] > bootstrap_scores[idj]) win++;
                        else if(bootstrap_scores[idi] < bootstrap_scores[idj]) loss++;
                        else tie++;
                    }
                    cout << "  " << eval_ids[k] << "\t"
                         << win/(double)bootstrap << " "
                         << tie/(double)bootstrap << " "
                         << loss/(double)bootstrap << endl;
                }
            }
        }
    }

}
