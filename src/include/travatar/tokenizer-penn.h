#ifndef TRAVATAR_TOKENIZER_PENN_H__
#define TRAVATAR_TOKENIZER_PENN_H__

// Note, some of this code was ported from the cicada decoder under the LGPL

#include <travatar/tokenizer.h>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <string>

namespace travatar {

// An interface that splits strings into space-separated tokens, or sentences
class TokenizerPenn : public Tokenizer {

protected:

      typedef boost::regex regex_type;
      typedef const char*  replace_type;
      typedef std::pair<regex_type, replace_type> pattern_type;
      typedef boost::unordered_map<std::string, std::string> StringMap;

      typedef std::vector<pattern_type, std::allocator<pattern_type> > pattern_set_type;

public:

    TokenizerPenn();

    virtual ~TokenizerPenn() { };

    virtual std::string Tokenize(const std::string & str);

protected:

    pattern_set_type patterns_;
    regex_type nonbreak_;
    StringMap replace_;


};

}

#endif
