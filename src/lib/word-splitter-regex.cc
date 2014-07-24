#include <travatar/word-splitter-regex.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

vector<string> WordSplitterRegex::StringSplit(const std::string & str,
                                        const std::string & pad) const {
    boost::sregex_iterator i(str.begin(), str.end(), profile_);
    boost::sregex_iterator j;
    int pos = 0;
    vector<string> ret;
    for(; i != j; ++i) {
        if(i->position() != pos)
            ret.push_back(str.substr(pos, i->position()-pos));
        string str = i->str();
        ret.push_back(pad+str+pad);
        pos = i->position() + str.length();
    }
    if(pos != (int)str.size())
        ret.push_back(str.substr(pos));

    return ret;
}

