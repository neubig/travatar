#include <iostream>
#include <fstream>
#include <travatar/util.h>
#include <travatar/mt-evaluator-runner.h>
#include <travatar/config-mt-evaluator-runner.h>
#include <travatar/dict.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void MTEvaluatorRunner::Run(const ConfigMTEvaluatorRunner & config) {

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
    algorithm::split(eval_ids, config.GetString("eval"), is_any_of(","));
    BOOST_FOREACH(const string & eval, eval_ids) {
        if(eval == "bleu") 
            eval_measures.push_back(shared_ptr<EvalMeasure>(new EvalMeasureBleu(4,0,EvalMeasureBleu::CORPUS)));
        else if(eval == "bleup1") 
            eval_measures.push_back(shared_ptr<EvalMeasure>(new EvalMeasureBleu(4,1,EvalMeasureBleu::SENTENCE)));
        else if(eval == "ribes")
            eval_measures.push_back(shared_ptr<EvalMeasure>(new EvalMeasureRibes));
        else
            THROW_ERROR("Unknown evaluation measure: " << config.GetString("eval"));
    }

    // Calculate the scores
    BOOST_FOREACH(const string & filename, config.GetMainArgs()) {
        vector<EvalStatsPtr> total_stats(eval_measures.size());
        int id = 0;
        ifstream sysin(filename.c_str());
        if(!sysin) THROW_ERROR("Could not open system file: " << filename);
        while(getline(sysin, line)) {
            Sentence sys_sent = Dict::ParseWords(line);
            for(int i = 0; i < (int)eval_measures.size(); i++) {
                if(total_stats[i].get() == NULL)
                    total_stats[i] = eval_measures[i]->CalculateStats(ref_sentences[0][id],sys_sent);
                else
                    total_stats[i]->PlusEquals(*eval_measures[i]->CalculateStats(ref_sentences[0][id], sys_sent));
            }
            id++;
        }
        int col = 0;
        // Print the evaluation for this file, with the filename if multiple files are being evaluated
        if(config.GetMainArgs().size() > 1) { cout << filename; col++; }
        if(id == ref_len) {
            BOOST_FOREACH(EvalStatsPtr stats, total_stats) {
                if(col++) cout << "\t";
                cout << stats->ConvertToString();
            }
        } else {
            if(col++) cout << "\t";
            cout << "FAILED: Reference (" << ref_len << ") and System (" << id << ") lengths don't match";
        }
        cout << endl;
    }
    

}
