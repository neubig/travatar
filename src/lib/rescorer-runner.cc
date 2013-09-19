#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <travatar/dict.h>
#include <travatar/config-rescorer-runner.h>
#include <travatar/rescorer-runner.h>
#include <travatar/util.h>
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
    
    // Finally, sort in descending order of weight
    sort(nbest.begin(), nbest.end());
}

// Print at least the top of the rescored n-best list
void RescorerRunner::Print(const RescorerNbest & nbest) {
    if(nbest.size() == 0) THROW_ERROR("Can not print an empty nbest");
    cout << Dict::PrintWords(nbest[0].sent) << endl;
}

// Run the model
void RescorerRunner::Run(const ConfigRescorer & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

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

    // Read in from the source
    istream * nbest_in;
    if(config.GetString("nbest").size() == 0 || config.GetString("nbest") == "-") {
        nbest_in = &cin;
    } else {
        nbest_in = new ifstream(config.GetString("nbest").c_str());
        if(!(*nbest_in)) THROW_ERROR("Could not find nbest file: " << config.GetString("nbest"));
    }

    // Load n-best lists
    regex threebars(" \\|\\|\\| ");
    RescorerNbest nbest;
    int last_id = -1;
    string line;
    while(getline(*nbest_in, line)) {
        vector<string> columns;
        algorithm::split_regex(columns, line, threebars);
        if(columns.size() != 4)
            THROW_ERROR("Expected 4 columns in n-best list:\n" << line);
        // Parse the values
        int id = boost::lexical_cast<int>(columns[0]);
        if(last_id != id && nbest.size() > 0) {
            Rescore(nbest);
            Print(nbest);
            nbest.clear();
        }
        last_id = id;
        nbest.push_back(RescorerNbestElement(Dict::ParseWords(columns[1]), 
                                             Dict::ParseFeatures(columns[3]),
                                             boost::lexical_cast<double>(columns[2])));
    }
    Rescore(nbest);
    Print(nbest);

}
