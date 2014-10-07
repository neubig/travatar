#include <travatar/tokenizer-penn.h>
#include <travatar/tokenizer-identity.h>
#include <travatar/tokenizer.h>
#include <travatar/global-debug.h>
#include <travatar/dict.h>

using namespace travatar;
using namespace std;

Sentence Tokenizer::TokenizeToSentence(const string & str) {
    return Dict::ParseWords(Tokenize(str));
}

Tokenizer* Tokenizer::CreateFromString(const string & str) {
    if(str == "none") {
        return new TokenizerIdentity();
    } else if(str == "penn") {
        return new TokenizerPenn();
    } else {
        THROW_ERROR("Unknown tokenizer type " << str);
    }
}
