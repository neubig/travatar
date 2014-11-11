#include <boost/foreach.hpp>
#include <travatar/translation-rule.h>
#include <travatar/sparse-map.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

void TranslationRule::Print(std::ostream & out) const {
    out << "{";
    if(trg_data_.size()) {
        out << "\"trg_data\": [";
        for(int i = 0; i < (int)trg_data_.size(); i++) {
            trg_data_[i].Print(out);
            out << ((i == (int)trg_data_.size()-1) ? "]" : ", ");
        }
    }
    if(features_.size()) {
        int pos = 0;
        if(trg_data_.size() != 0) out << ", ";
        out << "\"features\": {";
        BOOST_FOREACH(const SparsePair & val, features_.GetImpl()) {
            out << (pos++?", ":"") << "\""<<Dict::WSym(val.first)<<"\": " << val.second;
        }
        out << "}";
    }
    out << "}";
}

