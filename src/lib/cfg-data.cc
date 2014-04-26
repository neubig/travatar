#include <travatar/cfg-data.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

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
