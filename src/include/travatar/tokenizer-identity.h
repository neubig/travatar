#ifndef TRAVATAR_TOKENIZER_IDENTITY_H__
#define TRAVATAR_TOKENIZER_IDENTITY_H__

#include <travatar/tokenizer.h>
#include <string>

namespace travatar {

// An interface that splits strings into space-separated tokens, or sentences
class TokenizerIdentity : public Tokenizer {

public:
    virtual ~TokenizerIdentity() { };

    virtual std::string Tokenize(const std::string & str) { return str; }

};

}

#endif
