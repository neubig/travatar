#ifndef LOOKUP_TABLE_HIERO_H__
#define LOOKUP_TABLE_HIERO_H__

#include <vector>
#include <boost/shared_ptr.hpp>
#include <travatar/sparse-map.h>
#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>

using namespace boost;

namespace travatar {

typedef std::map<WordId, std::vector<TranslationRuleHiero*> > RuleMapHiero;

class LookupTableHiero {
public:
	virtual ~LookupTableHiero() { }
	
	static LookupTableHiero * ReadFromRuleTable(std::istream & in);

	static TranslationRuleHiero * BuildRule(travatar::TranslationRuleHiero * rule, std::vector<std::string> & source, 
			std::vector<std::string> & target, SparseMap features);

	virtual HyperGraph * BuildHyperGraph();

	
	// Inheritable functions
	virtual void AddRule(WordId rule_starting_word, TranslationRuleHiero * rule);
	virtual std::vector<TranslationRuleHiero*> & FindRules(WordId input);
	virtual std::string ToString();
protected:
	
	RuleMapHiero rule_map;
};

}

#endif