#include <travatar/tokenizer-penn.h>
#include <travatar/tokenizer-identity.h>
#include <travatar/tokenizer.h>
#include <travatar/global-debug.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;

TokenizerPenn::TokenizerPenn() {
    patterns_.push_back(pattern_type(regex_type("^[[:space:]]*\""), " `` "));
    
    patterns_.push_back(pattern_type(regex_type("([\\x20\\x28\\x5B\\x7B\\x3C])\""), "$1 `` "));
    
    patterns_.push_back(pattern_type(regex_type("([,;:@#$%&]|\\.\\.\\.\\.)"), " $1 "));
    
    patterns_.push_back(pattern_type(regex_type("([^.])([.])([\\x5B\\x5D\\x29\\x7D\\x3E\\x22\\x27]*)(?=[[:space:]]*$)"), "$1 $2$3"));
    patterns_.push_back(pattern_type(regex_type("([?!\\x5B\\x5D\\x28\\x29\\x7B\\x7D\\x3C\\x3E]|--)"), " $1 "));
    
    patterns_.push_back(pattern_type(regex_type("\""), " \'\' "));
    
    patterns_.push_back(pattern_type(regex_type("([^\'])\' "), "$1 \' ")); // possessive or close-single-quote
    patterns_.push_back(pattern_type(regex_type("\'([sSmMdD]|ll|LL|re|RE|ve|VE) "), " \'$1 ")); // as in it's I'm we'd, 'll 're
    patterns_.push_back(pattern_type(regex_type("(n\'t|N\'T) "), " $1 "));
    
    patterns_.push_back(pattern_type(regex_type(" ([Cc])annot "), " $1an not "));
    patterns_.push_back(pattern_type(regex_type(" ([Dd])\'ye "), " $1\' ye "));
    patterns_.push_back(pattern_type(regex_type(" ([Gg])imme "), " $1im me "));
    patterns_.push_back(pattern_type(regex_type(" ([Gg])onna "), " $1on na "));
    patterns_.push_back(pattern_type(regex_type(" ([Gg])otta "), " $1ot ta "));
    patterns_.push_back(pattern_type(regex_type(" ([Ll])emme "), " $1em me "));
    patterns_.push_back(pattern_type(regex_type(" ([Mm])ore\'n "), " $1ore \'n "));
    patterns_.push_back(pattern_type(regex_type(" \'([Tt])is "), " \'$1 is "));
    patterns_.push_back(pattern_type(regex_type(" \'([Tt])was "), " \'$1 was "));
    patterns_.push_back(pattern_type(regex_type(" ([Ww])anna "), " $1an na "));
    patterns_.push_back(pattern_type(regex_type("^ +"), ""));
    patterns_.push_back(pattern_type(regex_type(" +$"), ""));
    patterns_.push_back(pattern_type(regex_type("  +"), " "));
}

string TokenizerPenn::Tokenize(const string & str) {
    string tmp_str, ret = str;
    BOOST_FOREACH(const pattern_type & pat, patterns_) {
	  tmp_str.swap(ret);
	  ret.clear();
	  boost::regex_replace(std::back_inserter(ret),
			       tmp_str.begin(), tmp_str.end(),
			       pat.first, pat.second);
	}
    return ret;
}
