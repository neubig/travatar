#include <iostream>
#include <fstream>
#include <travatar/util.h>
#include <travatar/tree-io.h>
#include <travatar/rule-extractor.h>
#include <travatar/hiero-extractor-runner.h>
#include <travatar/config-hiero-extractor-runner.h>
#include <travatar/binarizer.h>
#include <travatar/rule-composer.h>
#include <travatar/rule-filter.h>
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/dict.h>
#include <boost/scoped_ptr.hpp>
#include <boost/tokenizer.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void HieroExtractorRunner::Run(const ConfigHieroExtractorRunner & config) {
    // Set the debugging level
    //GlobalVars::debug = config.GetInt("debug");
   
    // Create the rule extractor
    HieroExtractor extractor;
    extractor.SetMaxNonterm(config.GetInt("nonterm_len"));

    int term_len = config.GetInt("term_len");

    // Open the files
    const vector<string> & argv = config.GetMainArgs();
    ifstream src_in(argv[0].c_str());
    if(!src_in) THROW_ERROR("Could not find source file: " << argv[0]);
    ifstream trg_in(argv[1].c_str());
    if(!trg_in) THROW_ERROR("Could not find target file: " << argv[1]);
    ifstream alg_in(argv[2].c_str());
    if(!alg_in) THROW_ERROR("Could not find align file: " << argv[2]);
    
    string src_line,trg_line, align_line;
   

    long long int line = 0;
    while(true) {
		int has_src = getline(src_in,src_line) ? 1 : 0;
        int has_trg = getline(trg_in,trg_line) ? 1 : 0;
		int	has_align = getline(alg_in,align_line) ? 1 : 0;
		
		if (has_src+has_trg+has_align == 0) break;
        if (has_src+has_trg+has_align != 3) {
            THROW_ERROR("File sizes don't match.");
        }

        Alignment alignment = Alignment::FromString(align_line);
        Sentence src_sent = Dict::ParseWords(src_line);
        Sentence trg_sent = Dict::ParseWords(trg_line);

        std::vector<string> rule = extractor.ExtractHieroRule(alignment,src_sent,trg_sent);

        BOOST_FOREACH(string r , rule) {
            cout << r << endl;
        }
        if (++line % 1000 == 0) {
            cerr << "Finished Processing: " << line << " lines. " << endl; 
        }
    }
}
