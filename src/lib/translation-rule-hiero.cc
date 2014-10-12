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
    src_data_(src_data)
{ }

void TranslationRuleHiero::Print(std::ostream & out) const {
   out << Dict::PrintAnnotatedWords(src_data_) << " ||| " << Dict::PrintAnnotatedVector(trg_data_);
}

HieroHeadLabels TranslationRuleHiero::GetHeadLabels() const {
    HieroHeadLabels ret(trg_data_.size()+1);
    ret[0] = src_data_.label;
    for(int i = 0; i < (int)trg_data_.size(); i++)
        ret[i+1] = trg_data_[i].label;
    return ret;
}

HieroHeadLabels TranslationRuleHiero::GetChildHeadLabels(int pos) const {
    HieroHeadLabels ret(trg_data_.size()+1);
    ret[0] = src_data_.syms[pos];
    for(int i = 0; i < (int)trg_data_.size(); i++)
        ret[i+1] = trg_data_[i].syms[pos];
    return ret;
}
