
#include <iostream>
#include <travatar/alignment.h>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace travatar;

// Convert to and from strings
string Alignment::ToString() const {
    ostringstream oss;
    for(int i = 0; i < (int)vec_.size(); i++) {
        if(i != 0) oss << " ";
        oss << vec_[i].first << "-" << vec_[i].second;
    }
    return oss.str();
}
Alignment::AlignmentPair Alignment::SplitAlignment(const string & str) {
    vector<string> left_right;
    algorithm::split(left_right, str, is_any_of("-"));
    if(left_right.size() != 2)
        THROW_ERROR("Bad alignment string (must be in the format \"LENGTH ||| FPOS1-EPOS1 FPOS1-EPOS1\"" << endl << str);
    AlignmentPair ret;
    istringstream left(left_right[0]); left >> ret.first;
    istringstream right(left_right[1]); right >> ret.second;
    return ret;
}
Alignment Alignment::FromString(const string & str) {
    Alignment ret;
    tokenizer<char_separator<char> > aligns(str, char_separator<char>(" "));
    BOOST_FOREACH(string str, aligns)
        ret.AddAlignment(SplitAlignment(str));
    return ret;
}
