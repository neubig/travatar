#include <boost/foreach.hpp>
#include <travatar/translation-rule-hiero.h>
#include <travatar/sparse-map.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

void TranslationRuleHiero::AddSourceWord (WordId id, std::pair<int,int> span) {
	source_sent.push_back(id);
	source_span.push_back(span);
}

void TranslationRuleHiero::AddTargetWord (WordId id, std::pair<int,int> span) {
	target_sent.push_back(id);
	target_span.push_back(span);
}

void TranslationRuleHiero::AddFeature(int id, double feat) {
	features.insert(make_pair<int,double>(id,feat));
}

void TranslationRuleHiero::AddFeature(const std::string & str, double feat) {
	features.insert(make_pair<int,double>(Dict::WID(str),feat));
}

void TranslationRuleHiero::SetFeatures(SparseMap & features_) {
	features = features_;
}