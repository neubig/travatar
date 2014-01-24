#include <boost/foreach.hpp>
#include <travatar/translation-rule-hiero.h>
#include <travatar/sparse-map.h>
#include <sstream>
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

string TranslationRuleHiero::ToString() {
	std::ostringstream ss;
	for (int i=0; i < (int)source_sent.size(); ++i) {
		if (i) ss << " ";
		if(source_sent[i] > 0) 
			ss << Dict::WSym(source_sent[i]);
		else 
			ss << "x" << (-source_sent[i])-1;
		ss << "[" << source_span[i].first << "," << source_span[i].second << "]";
	}
	ss << " ||| ";
	for (int i=0; i < (int)target_sent.size(); ++i) {
		if (i) ss << " ";
		if (target_sent[i] > 0)
			ss << Dict::WSym(target_sent[i]);
		else 
			ss << "x" << (-target_sent[i])-1;
		ss << "[" << target_span[i].first << "," << target_span[i].second << "]";
	}
	return ss.str();
}