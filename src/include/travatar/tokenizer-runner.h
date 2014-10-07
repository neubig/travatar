#ifndef TOKENIZER_RUNNER_H__ 
#define TOKENIZER_RUNNER_H__

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <travatar/sentence.h>

namespace travatar {

class ConfigTokenizerRunner;

// A class to perform tokenization
class TokenizerRunner {

public:

    TokenizerRunner() { }
    ~TokenizerRunner() { }
    
    // Run the tokenizer
    void Run(const ConfigTokenizerRunner & config);

};

}

#endif

