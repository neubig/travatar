#include <boost/foreach.hpp>
#include <travatar/translation-rule-hiero.h>
#include <travatar/sparse-map.h>
#include <travatar/dict.h>
#include <sstream>

using namespace std;
using namespace travatar;

TranslationRuleHiero::TranslationRuleHiero(
        const CfgDataVector & trg_data,
        const SparseVector & features,
        const CfgData & src_data
        ) : TranslationRule(trg_data, features),
    src_data_(src_data), head_labels_(), child_head_labels_()
{
    ostringstream oss;
    // Head Symbols
    vector<WordId> head_label;
    head_label.push_back(src_data.label);
    oss << Dict::WSym(src_data.label);
    for (int i=0; i < (int) trg_data.size(); ++i) {
        head_label.push_back(trg_data[i].label);
        oss << "," << Dict::WSym(trg_data[i].label);
    }
    head_labels_string_ = Dict::WID(oss.str());
    head_labels_ = GenericString<WordId>(head_label);
    int j=0;
    do {
        int label = src_data.GetSym(j);
        if (label == -1) 
            break;
        head_label.clear();
        head_label.push_back(label);
        for (int k=0; k < (int) trg_data.size(); ++k) 
            head_label.push_back(trg_data[k].GetSym(j));
        child_head_labels_.push_back(GenericString<WordId>(head_label));
        ++j;
    } while (1);
}


void TranslationRuleHiero::Print(std::ostream & out) const {
   out << Dict::PrintAnnotatedWords(src_data_) << " ||| " << Dict::PrintAnnotatedVector(trg_data_);
}


//string TranslationRuleHiero::ToString() {
//    std::ostringstream ss;
//    ss << Dict::PrintAnnotatedWords(src_data_) << " ||| " << Dict::PrintAnnotatedVector(trg_data_);
//    return ss.str();
//}
