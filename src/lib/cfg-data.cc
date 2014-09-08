#include <travatar/cfg-data.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace travatar;

const std::vector<int> CfgData::GetNontermPositions() const {
    std::vector<int> ret;
    for(int i = 0; i < (int)words.size(); i++)
        if(words[i] < 0)
            ret.push_back(i);
    return ret;
}

void CfgData::AppendChild(const CfgData & child) {
    BOOST_FOREACH(WordId wid, child.words)
        words.push_back(wid);
    syms.push_back(child.label);
}

void CfgData::Print(std::ostream & out) const {
    out << "{\"label\": " << label;
    if(words.size()) {
        out << ", \"words\": [";
        for(int j = 0; j < (int)words.size(); j++)
            out << Dict::WSymEscaped(words[j]) << (j == (int)words.size()-1 ? "]" : ", ");
    }
    if(syms.size()) {
        out << ", \"syms\": [";
        for(int j = 0; j < (int)syms.size(); j++)
            out << Dict::WSymEscaped(syms[j]) << (j == (int)syms.size()-1 ? "]" : ", ");
    }
    out << "}";
}
