#ifndef TRAVATAR_TOKENIZER_H__
#define TRAVATAR_TOKENIZER_H__

#include <travatar/sentence.h>
#include <string>

namespace travatar {

// An interface that splits strings into space-separated tokens, or sentences
class Tokenizer {

public:
    virtual ~Tokenizer() { };

    virtual std::string Tokenize(const std::string & str) = 0;
    virtual Sentence TokenizeToSentence(const std::string & str);

    static Tokenizer* CreateFromString(const std::string & str);

};

}

#endif
