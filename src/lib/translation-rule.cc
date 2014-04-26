#include <boost/foreach.hpp>
#include <travatar/translation-rule.h>
#include <travatar/sparse-map.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

void TranslationRule::Print(std::ostream & out) const {
    out << "{\"src\": \""<<src_str_ << "\"";
    if(trg_data_.size()) {
        out << ", \"trg_data\": [";
        for(int i = 0; i < (int)trg_data_.size(); i++) {
            trg_data_[i].Print(out);
            out << ((i == (int)trg_data_.size()-1) ? "]" : ", ");
        }
    }
    if(features_.size()) {
        int pos = 0;
        out << ", \"features\": {";
        BOOST_FOREACH(const SparsePair & val, features_) {
            out << (pos++?", ":"") << "\""<<Dict::WSym(val.first)<<"\": " << val.second;
        }
        out << "}";
    }
    out << "}";
}

void TranslationRule::AddFeature(int id, double feat) { 
    features_.insert(make_pair(id, feat));
}
void TranslationRule::AddFeature(const std::string & str, double feat) { 
    features_.insert(make_pair(Dict::WID(str), feat));
}
