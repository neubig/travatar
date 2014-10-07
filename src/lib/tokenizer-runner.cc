
#include <travatar/config-tokenizer-runner.h>
#include <travatar/tokenizer-runner.h>
#include <travatar/tokenizer.h>
#include <travatar/global-debug.h>
#include <boost/scoped_ptr.hpp>
#include <iostream>

using namespace travatar;
using namespace std;

// Run the model
void TokenizerRunner::Run(const ConfigTokenizerRunner & config) {

    // Set the global variables
    GlobalVars::debug = config.GetInt("debug");
    boost::scoped_ptr<Tokenizer> tokenizer(Tokenizer::CreateFromString(config.GetString("type")));

    string line;
    while(getline(cin, line)) {
        cout << tokenizer->Tokenize(line) << endl;
    }

}
