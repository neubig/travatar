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

	virtual HyperGraph * BuildHyperGraph(string input);

	
	// Inheritable functions
	virtual void AddRule(WordId rule_starting_word, TranslationRuleHiero * rule);
	virtual std::vector<TranslationRuleHiero*> & FindRules(WordId input);
	virtual std::string ToString();
protected:
	
	RuleMapHiero rule_map;

private:
	int Hash(int x, int y) { return 1000000*x + y; }
	std::pair<int,int> Dehash(int value) { return std::make_pair<int,int>(value / 1000000, value % 1000000);}
	HyperNode* FindNode(map<int, HyperNode*> & _map, int begin, int end);

};

}

#endif