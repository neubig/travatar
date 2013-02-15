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
    if(config.GetString("eval") == "bleup1") 
        eval_meas.reset(new EvalMeasureBleu);
    else
        THROW_ERROR("Unknown evaluation measure: " << config.GetString("eval"));

    // Calculate the scores
    BOOST_FOREACH(const string & filename, config.GetMainArgs()) {
        int id = 0;
        double total_eval = 0;
        ifstream sysin(filename.c_str());
        if(!sysin) THROW_ERROR("Could not open system file: " << filename);
        while(getline(sysin, line)) {
            Sentence sys_sent = Dict::ParseWords(line);
            double max_eval = 0;
            BOOST_FOREACH(const vector<Sentence> & refs, ref_sentences) {
                max_eval = max(max_eval, eval_meas->MeasureScore(refs[id],sys_sent));
            }
            id++;
            total_eval += max_eval;
        }
        if(config.GetMainArgs().size() > 1) cout << filename << ": ";
        cout << total_eval/id << endl;
    }
    

}
