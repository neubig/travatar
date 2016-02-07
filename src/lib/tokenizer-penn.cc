#include <travatar/tokenizer-penn.h>
#include <travatar/tokenizer-identity.h>
#include <travatar/tokenizer.h>
#include <travatar/global-debug.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

using namespace travatar;
using namespace std;

TokenizerPenn::TokenizerPenn() {
    patterns_.push_back(pattern_type(regex_type("^[[:space:]]*[\'`]"), " ` "));
    patterns_.push_back(pattern_type(regex_type("([\\x20\\x28\\x5B\\x7B\\x3C])[\'`]"), "$1 ` "));
    patterns_.push_back(pattern_type(regex_type("^[[:space:]]*\""), " `` "));
    patterns_.push_back(pattern_type(regex_type("([\\x20\\x28\\x5B\\x7B\\x3C])\""), "$1 `` "));
    
    patterns_.push_back(pattern_type(regex_type("([;@#$%&=*]+|\\.\\.\\.)"), " $1 "));
    patterns_.push_back(pattern_type(regex_type("([^0-9])([,:])"), "$1 $2 "));
    patterns_.push_back(pattern_type(regex_type("([,:])([^0-9])"), " $1 $2"));
    
    // patterns_.push_back(pattern_type(regex_type("([^.])([.])([\\x5B\\x5D\\x29\\x7D\\x3E\\x22\\x27]*)(?=[[:space:]]*$)"), "$1 $2$3"));
    patterns_.push_back(pattern_type(regex_type("([.])([\\x5B\\x5D\\x29\\x7D\\x3E\\x22\\x27]+)"), "$1 $2"));
    
    // Last two are unicode quotes
    patterns_.push_back(pattern_type(regex_type("([?!\\x5B\\x5D\\x28\\x29\\x7B\\x7D\\x3C\\x3E]|--|\\xE2\\x80\\x9C|\\xE2\\x80\\x9D)"), " $1 "));
    
    patterns_.push_back(pattern_type(regex_type("\""), " \'\' "));
    
    patterns_.push_back(pattern_type(regex_type("([^\'])\'([ ,.])"), "$1 \'$2")); // possessive or close-single-quote
    patterns_.push_back(pattern_type(regex_type("\'([sSmMdD]|ll|LL|re|RE|ve|VE)([ ,.\"'])"), " \'$1$2")); // as in it's I'm we'd, 'll 're
    patterns_.push_back(pattern_type(regex_type("(n\'t|N\'T)([ ,.\"'])"), " $1$2"));
    patterns_.push_back(pattern_type(regex_type(" (No|Vol|vol|no|nos|Fig|fig|Tab|tab|pp|c)\\.([0-9])"), " $1. $2"));
    patterns_.push_back(pattern_type(regex_type("([0-9])(|T|G|M|k|h|da|d|c|m|n|p)(m|g|l|A|K|mol|cd|rad|sr|Hz|N|Pa|J|W|C|V|eV|F|Ω|S|Wb|T|H|lm|lx|Bq|Gy|Sv|kat|rpm|Wd|dB|db) "), "$1 $2$3 "));

    nonbreak_ = regex_type("(A|B|C|D|E|F|G|H|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|Jan|Feb|Mar|Apr|Jun|Jul|Aug|Sep|Sept|Oct|Dec|Adj|Adm|Adv|Asst|Bart|Bldg|Brig|Bros|Capt|Cmdr|Co|CO|Col|Comdr|Con|Corp|Cpl|Dept|DR|Dr|Drs|Ens|Gen|Gov|Hon|Hr|Hosp|Inc|INC|Insp|Lt|Ltd|LTD|MM|MR|MRS|MS|Maj|Messrs|Mlle|Mme|Mr|Mrs|Ms|Msgr|Mt|No|Op|Ord|Pfc|Ph|Prof|Pvt|Rep|Reps|Res|Rev|Rt|Sen|Sens|Sfc|Sgt|Sr|St|Supt|Surg|v|vs|rev|Nos|Nr|etc|al|c|nos|ca|cf|m|p|ed|No|Vol|vol|no|nos|Fig|fig|Tab|tab|pp|approx|\\.\\.)[.]");

    replace_["…"] = "...";
    replace_["–"] = "--";
    replace_[".'"] = ". '";
    replace_["Cannot"] = "Can not";
    replace_["D'ye"] = "D' ye";
    replace_["Gimme"] = "Gim me";
    replace_["Gonna"] = "Gon na";
    replace_["Gotta"] = "Got ta";
    replace_["Lemme"] = "Lem me";
    replace_["More'n"] = "More 'n";
    replace_["'Tis"] = "'T is";
    replace_["'Twas"] = "'T was";
    replace_["Wanna"] = "Wan na";
    replace_["cannot"] = "can not";
    replace_["d'ye"] = "d' ye";
    replace_["gimme"] = "gim me";
    replace_["gonna"] = "gon na";
    replace_["gotta"] = "got ta";
    replace_["lemme"] = "lem me";
    replace_["more'n"] = "more 'n";
    replace_["'tis"] = "'t is";
    replace_["'twas"] = "'t was";
    replace_["wanna"] = "wan na";
    replace_["("] = "-LRB-";
    replace_[")"] = "-RRB-";
    replace_["["] = "-LSB-";
    replace_["]"] = "-RSB-";
    replace_["{"] = "-LCB-";
    replace_["}"] = "-RCB-";
}

string TokenizerPenn::Tokenize(const string & str) {
    string tmp_str, ret = " "+str+" ";
    BOOST_FOREACH(const pattern_type & pat, patterns_) {
	  tmp_str.swap(ret);
	  ret.clear();
	  boost::regex_replace(std::back_inserter(ret),
			       tmp_str.begin(), tmp_str.end(),
			       pat.first, pat.second);
	} 
    vector<string> words, ret_words;
    boost::algorithm::split(words,ret,boost::is_any_of(" "),boost::token_compress_on); 
    ret_words.reserve(words.size());
    BOOST_FOREACH(const std::string word, words) {
        if(word != "") {
            if(word[word.length()-1] == '.') {
                if(word.length() > 1 && word.find('.') == word.length()-1 && !boost::regex_match(word, nonbreak_)) {
                    ret_words.push_back(word.substr(0, word.length()-1) + " .");
                } else {
                    ret_words.push_back(word);
                }
            } else {
                StringMap::const_iterator it = replace_.find(word);
                ret_words.push_back(it == replace_.end() ? word : it->second);
            }
        }
    }
    return boost::algorithm::join(ret_words, " ");
}
