#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <travatar/dict.h>
#include <travatar/config-rescorer-runner.h>
#include <travatar/rescorer-runner.h>
#include <travatar/input-file-stream.h>
#include <travatar/util.h>
#include <travatar/eval-measure.h>
#include <travatar/hyper-graph.h>
#include <travatar/tree-io.h>
#include <travatar/output-collector.h>

using namespace travatar;
using namespace std;
using namespace boost;

// Rescore an n-best list
void RescorerRunner::Rescore(RescorerNbest & nbest) {
    // If we have weights, rescore based on the weights
    if(rescore_weights_) {
        BOOST_FOREACH(RescorerNbestElement & elem, nbest)
            elem.score = elem.feat * weights_;
    }
    
    // If we are doing MBR rescoring
    if(mbr_eval_.get() != NULL) {

        // Reduce the number of hypotheses before MBR
        if(mbr_hyp_cnt_ && (int)nbest.size() > mbr_hyp_cnt_) {
            sort(nbest.begin(), nbest.end());
            nbest.resize(mbr_hyp_cnt_);
        }

        // First get the maximum probability
        double max_score = -DBL_MAX;
        BOOST_FOREACH(RescorerNbestElement & elem, nbest)
            max_score = max(max_score, elem.score);
        // Convert to probabilities and and sum
        double sum = 0;
        BOOST_FOREACH(RescorerNbestElement & elem, nbest) {
            elem.score = exp( (elem.score-max_score)*mbr_scale_ );
            sum += elem.score;
        }

        // A map with the probability/expected BLEU of each sentence
        typedef pair<const Sentence, pair<double,double> > SentProbExp; 
        std::map<Sentence, pair<double,double> > prob_exp; 

        // Calculate probabilities
        for(int i = 0; i < (int)nbest.size(); i++)
            prob_exp[nbest[i].sent].first += nbest[i].score/sum;
        PRINT_DEBUG("Sentence " << sent_ << " MBR hypotheses: " << prob_exp.size() << endl, 1);

        // Calculate the expectation for each sentence
        int si = 0;
        BOOST_FOREACH(SentProbExp & hyp, prob_exp) {
            int sj = 0;
            BOOST_FOREACH(SentProbExp & ref, prob_exp) {
                hyp.second.second += ref.second.first * 
                    mbr_eval_->CalculateCachedStats(ref.first,hyp.first,sj,si)->ConvertToScore();
                sj++;
            }
            si++;
        }
        mbr_eval_->ClearCache();
        
        // Apply these to the actual scores
        BOOST_FOREACH(RescorerNbestElement & elem, nbest)
            elem.score = prob_exp[elem.sent].second;

    }

    // Finally, sort in descending order of weight
    sort(nbest.begin(), nbest.end());

}

// Print at least the top of the rescored n-best list
void RescorerRunner::Print(const RescorerNbest & nbest) {
    if(nbest.size() == 0) THROW_ERROR("Can not print an empty nbest");
    cout << Dict::PrintWords(nbest[0].sent) << endl;
    if(nbest_out_.get() != NULL) {
        BOOST_FOREACH(const RescorerNbestElement & elem, nbest) {
            *nbest_out_ << sent_ << " ||| "
                        << Dict::PrintWords(elem.sent) << " ||| "
                        << elem.score << " ||| "
                        << Dict::PrintFeatures(elem.feat) << endl;
        } 
    }
}

// Run the model
void RescorerRunner::Run(const ConfigRescorer & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Read in from the source
    istream * nbest_in;
    if(config.GetString("nbest").size() == 0 || config.GetString("nbest") == "-") {
        nbest_in = &cin;
    } else {
        nbest_in = new InputFileStream(config.GetString("nbest").c_str());
        if(!(*nbest_in)) THROW_ERROR("Could not find nbest file: " << config.GetString("nbest"));
    }

    // Open the nbest output file if necessary
    if(config.GetString("nbest_out").size() != 0) {
        nbest_out_.reset(new ofstream(config.GetString("nbest_out").c_str()));
        if(!(*nbest_out_)) THROW_ERROR("Could not open nbest out: " << config.GetString("nbest_out"));
    }

    // If we have weights, read in the weights file
    if(config.GetString("weight_in") != "") {
        cerr << "Reading weight file from "<<config.GetString("weight_in")<<"..." << endl;
        ifstream weight_in(config.GetString("weight_in").c_str());
        if(!weight_in)
            THROW_ERROR("Could not find weights: " << config.GetString("weight_in"));
        weights_ = Dict::ParseFeatures(weight_in);
        rescore_weights_ = true;
        weight_in.close();
    }

    // Create an evaluation measure for MBR if necessary
    if(config.GetString("mbr_eval") != "") {
        mbr_eval_.reset(EvalMeasure::CreateMeasureFromString(config.GetString("mbr_eval")));
        mbr_scale_ = config.GetDouble("mbr_scale");
        mbr_hyp_cnt_ = config.GetInt("mbr_hyp_cnt");
    }

    // Load n-best lists
    RescorerNbest nbest;
    int last_id = -1;
    string line;
    while(getline(*nbest_in, line)) {
        vector<string> columns = Tokenize(line, " ||| ");
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        // Parse the values
        int id = boost::lexical_cast<int>(columns[0]);
        if(last_id != id && nbest.size() > 0) {
            Rescore(nbest);
            Print(nbest);
            nbest.clear();
            sent_++;
        }
        last_id = id;
        nbest.push_back(RescorerNbestElement(Dict::ParseWords(columns[1]), 
                                             Dict::ParseFeatures(columns[3]),
                                             boost::lexical_cast<double>(columns[2])));
    }
    Rescore(nbest);
    Print(nbest);

}
