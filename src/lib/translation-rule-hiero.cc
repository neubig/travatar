#include <boost/foreach.hpp>
#include <travatar/translation-rule-hiero.h>
#include <travatar/sparse-map.h>
#include <sstream>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

void TranslationRuleHiero::AddSourceWord (WordId id) {
	if (id < 0) {
		++n_term;
		non_term_position.push_back(source_sent.size());
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
	for (int i=0; i < (int)source_sent.size(); ++i) {
		if (i) ss << " ";
		if(source_sent[i] > 0) 
			ss << Dict::WSym(source_sent[i]);
		else 
			ss << "x" << (-source_sent[i])-1;
	}
	ss << " ||| ";
	for (int i=0; i < (int)trg_words_.size(); ++i) {
		if (i) ss << " ";
		if (trg_words_[i] > 0)
			ss << Dict::WSym(trg_words_[i]);
		else 
			ss << "x" << (-trg_words_[i])-1;
	}
	ss << " [";
	std::pair<int,int> prs;
	BOOST_FOREACH(prs, span_vector) {
		ss << "(" << prs.first << "," << prs.second << ")";
	}
	ss << "]"; 
	return ss.str();
}