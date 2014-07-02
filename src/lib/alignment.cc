
#include <iostream>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <travatar/alignment.h>
#include <travatar/util.h>

using namespace std;
using namespace boost;
using namespace travatar;

// Convert to and from strings
string Alignment::ToString() const {
    ostringstream oss;
    int cnt = 0;
    BOOST_FOREACH(AlignmentPair align, vec_) {
        if(cnt++ != 0) oss << " ";
        oss << align.first << "-" << align.second;
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

void Alignment::AddAlignment(const AlignmentPair & al) {
#ifdef LADER_SAFE
    if(al.first >= len_.first || al.second >= len_.second)
        THROW_ERROR("Out of bounds in AddAlignment: "<< al << ", " << len_);
#endif
    vec_.insert(al);
}

std::vector<std::set<int> > Alignment::GetSrcAlignments() const {
    std::vector<std::set<int> > ret;
    BOOST_FOREACH(AlignmentPair al, vec_) {
        if((int)ret.size() <= al.first) ret.resize(al.first+1);
        ret[al.first].insert(al.second);
    }
    return ret;
}
