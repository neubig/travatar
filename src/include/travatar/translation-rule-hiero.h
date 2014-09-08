#ifndef TRANSLATION_RULE_HIERO_H__
#define TRANSLATION_RULE_HIERO_H__

#include <travatar/translation-rule.h>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <travatar/generic-string.h>
#include <string>
#include <vector>

namespace travatar {
typedef GenericString<WordId> HieroHeadLabels;
class TranslationRuleHiero : public TranslationRule {
public: 
    TranslationRuleHiero(
        const CfgDataVector & trg_data = CfgDataVector(),
        const SparseVector & features = SparseVector(),
        const CfgData & src_data = Sentence()
        );

    virtual void Print(std::ostream & out) const;
   
    virtual bool operator==(const TranslationRuleHiero & rhs) const {
        return
            trg_data_ == rhs.trg_data_ &&
            features_ == rhs.features_ &&
            src_data_ == rhs.src_data_;
    }

    // ACCESSOR
    const CfgData & GetSrcData() const { return src_data_; }
    const HieroHeadLabels & GetHeadLabels() const { return head_labels_; }
    const HieroHeadLabels & GetChildHeadLabels(int position) const { return child_head_labels_[position]; } 
    WordId GetHeadLabelsString() const { return head_labels_string_; }
protected:
	CfgData src_data_;
    HieroHeadLabels head_labels_;
    std::vector<HieroHeadLabels > child_head_labels_;
    WordId head_labels_string_;
};
}
#endif
