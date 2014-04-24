#include <boost/foreach.hpp>
#include <travatar/translation-rule-hiero.h>
#include <travatar/sparse-map.h>
#include <sstream>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

void TranslationRuleHiero::AddSourceWord (WordId id, WordId label) {
	if (id < 0) {
		non_term_position.push_back(source_sent.size());
		if (label != 0) {
			non_term_label.push_back(label);
		}
	}
	source_sent.push_back(id);
}

void TranslationRuleHiero::SetFeatures(SparseMap & features_) {
	std::pair<int,double> ptr;
	BOOST_FOREACH(ptr, features_) {
		AddFeature(ptr.first, ptr.second);
	}
}

string TranslationRuleHiero::ToString() {
	std::ostringstream ss;
	for (int i=0, j=0; i < (int)source_sent.size(); ++i) {
		if (i) ss << " ";
		if(source_sent[i] >= 0) {
			ss << "\"" << Dict::WSym(source_sent[i]) << "\"";
		} else {
			ss << "x" << (-source_sent[i])-1;
			if (non_term_label.size() != 0) ss << ":" << Dict::WSym(GetChildNTLabel(j++));
		}
	}
	ss << " ||| ";
	for (int i=0; i < (int)trg_words_.size(); ++i) {
		if (i) ss << " ";
		if (trg_words_[i] >= 0){
			ss << "\"" << Dict::WSym(trg_words_[i]) << "\"";
		} else {
			ss << "x" << (-trg_words_[i])-1;
		}
	}
	if (label_ != -1) ss << " @ " << Dict::WSym(GetLabel());
	return ss.str();
}
