#ifndef TRANSLATION_RULE_HIERO_H__
#define TRANSLATION_RULE_HIERO_H__

#include <string>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/sparse-map.h>
#include <travatar/translation-rule.h>

using namespace std;

namespace travatar {
class TranslationRuleHiero : public TranslationRule {
public: 
	TranslationRuleHiero() { }

	void AddSourceWord (WordId id, WordId label=0);

    void SetFeatures(SparseMap & features);
    void SetSrcStr(std::string src_str) { src_str_ = src_str; }
    void SetLabel(WordId label) { label_ = label; }
    string ToString();

    virtual bool operator==(const TranslationRuleHiero & rhs) const {
        return
            trg_words_ == rhs.trg_words_ &&
            features_ == rhs.features_ &&
            source_sent == rhs.source_sent;
    }

    // ACCESSOR
    Sentence & GetSourceSentence() { return source_sent; }
    std::vector<int> & GetNonTermPositions() { return non_term_position; }
    WordId GetLabel() { return label_; }
    WordId GetChildNTLabel(int position) { 
        return non_term_label[position]; 
    }
protected:
    WordId label_;
	Sentence source_sent;
    std::vector<int> non_term_position;
    std::vector<WordId> non_term_label;
};

}
#endif