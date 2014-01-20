#ifndef TRANSLATION_RULE_HIERO_H__
#define TRANSLATION_RULE_HIERO_H__

#include <string>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/sparse-map.h>

using namespace std;

namespace travatar {
class TranslationRuleHiero {
public: 
	TranslationRuleHiero() {}
	~TranslationRuleHiero() {}

	void AddSourceWord (WordId id, std::pair<int,int> span);
	void AddTargetWord (WordId id, std::pair<int,int> span);
	void AddFeature(int id, double feat);
    void AddFeature(const std::string & str, double feat);
    void SetFeatures(SparseMap & features);
    
    // ACCESSOR
    Sentence & GetSourceSentence() { return source_sent; }
    Sentence & GetTargetSentence() { return target_sent; }

    // COMPARATOR
    bool operator==(const TranslationRuleHiero & rhs) const {
        return
            source_sent == rhs.source_sent &&
            target_sent == rhs.target_sent &&
            source_span == rhs.source_span &&
            target_span == rhs.target_span &&
            features == rhs.features;
    }
    bool operator!=(const TranslationRuleHiero & rhs) const {
        return !(*this == rhs);
    }

protected:
	Sentence source_sent;
	Sentence target_sent;
	std::vector<pair<int,int> > source_span;
	std::vector<pair<int,int> > target_span;
	SparseMap features;
};

}
#endif