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
	TranslationRuleHiero() { n_term = 0; }

	void AddSourceWord (WordId id);
    void SetFeatures(SparseMap & features);
    void SetSrcStr(std::string src_str) { src_str_ = src_str; }
    string ToString();

    virtual bool operator==(const TranslationRuleHiero & rhs) const {
        return
            n_term == rhs.n_term &&
            trg_words_ == rhs.trg_words_ &&
            features_ == rhs.features_ &&
            source_sent == rhs.source_sent;
    }

    bool CheckNTSourceTargetEqual();

    // ACCESSOR
    Sentence & GetSourceSentence() { return source_sent; }
    int GetNumberOfNonTerminals() { return n_term; }
    std::vector<int> & GetNonTermPositions() { return non_term_position; }
protected:
    int n_term;
	Sentence source_sent;
    std::vector<int> non_term_position;
};

}
#endif