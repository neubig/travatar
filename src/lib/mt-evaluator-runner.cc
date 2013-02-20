#include <iostream>
#include <fstream>
#include <travatar/util.h>
#include <travatar/mt-evaluator-runner.h>
#include <travatar/config-mt-evaluator-runner.h>
#include <travatar/dict.h>
#include <travatar/eval-measure-bleu.h>
#include <boost/shared_ptr.hpp>

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
    
    // Load the evaluation measure
    shared_ptr<EvalMeasure> eval_meas;
    if(config.GetString("eval") == "bleu") 
        eval_meas.reset(new EvalMeasureBleu(4,0,EvalMeasureBleu::CORPUS));
    else if(config.GetString("eval") == "bleup1") 
        eval_meas.reset(new EvalMeasureBleu(4,1,EvalMeasureBleu::SENTENCE));
    else
        THROW_ERROR("Unknown evaluation measure: " << config.GetString("eval"));

    // Calculate the scores
    BOOST_FOREACH(const string & filename, config.GetMainArgs()) {
        EvalStatsPtr total_stats;
        int id = 0;
        ifstream sysin(filename.c_str());
        if(!sysin) THROW_ERROR("Could not open system file: " << filename);
        while(getline(sysin, line)) {
            Sentence sys_sent = Dict::ParseWords(line);
            if(total_stats.get() == NULL)
                total_stats = eval_meas->CalculateStats(ref_sentences[0][id],sys_sent);
            else
                total_stats->PlusEquals(*eval_meas->CalculateStats(ref_sentences[0][id], sys_sent));
            id++;
        }
        if(config.GetMainArgs().size() > 1) cout << filename << ": ";
        cout << total_stats->ConvertToString() << endl;
    }
    

}
