#ifndef LOOKUP_TABLE_FSM_H__
#define LOOKUP_TABLE_FSM_H__

#include <vector>
#include <boost/shared_ptr.hpp>
#include <travatar/sparse-map.h>
#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/graph-transformer.h>
#include <generic-string.h>

using namespace boost;
using namespace std;

namespace travatar {
class LookupNodeFSM {
typedef std::map<WordId, LookupNodeFSM*> NodeMap;
public:
	LookupNodeFSM() { 
		lookup_map = NodeMap(); 
	}

	virtual ~LookupNodeFSM() { 
		BOOST_FOREACH(NodeMap::value_type &it, lookup_map) {
			delete it.second++;
		}
		BOOST_FOREACH(TranslationRuleHiero* rule, rules) {
			delete rule;
		}
	}
	
	virtual void AddEntry(WordId & key, LookupNodeFSM* rule);
	virtual LookupNodeFSM* FindNode(WordId key) const;
	virtual void AddRule(TranslationRuleHiero* rule);
	virtual std::string ToString() const;
	std::vector<TranslationRuleHiero*> & GetTranslationRules() { return rules; }
	
protected:
	NodeMap lookup_map;
	std::vector<TranslationRuleHiero*> rules;
private:
	std::string ToString(int indent) const;
};	

class LookupTableFSM : public GraphTransformer {
typedef std::vector<std::pair<int,int> > HieroRuleSpans;
typedef map<pair<int,int>, HyperNode*> NodeMap;
typedef GenericString<int> EdgeKey;
typedef set<EdgeKey > EdgeSet;
public:
	LookupTableFSM() {
		root_node = new LookupNodeFSM;
		glue_rule = new TranslationRuleHiero();
		SparseMap features = Dict::ParseFeatures("glue=0.5");
        glue_rule->SetFeatures(features);
        glue_rule->AddSourceWord(-1);
        glue_rule->AddSourceWord(-2);
        glue_rule->AddTrgWord(-1);
        glue_rule->AddTrgWord(-2);
        glue_rule->SetSrcStr("x0 x1");
        span_length = 20;
	}

	virtual ~LookupTableFSM() { 
		delete root_node;
		delete glue_rule;
	}
	
	static LookupTableFSM * ReadFromRuleTable(std::istream & in);

	static TranslationRuleHiero * BuildRule(travatar::TranslationRuleHiero * rule, std::vector<std::string> & source, 
			std::vector<std::string> & target, SparseMap features);

	virtual void AddRule(TranslationRuleHiero* rule);

	virtual std::string ToString() const;

	virtual HyperGraph * TransformGraph(const HyperGraph & graph) const;

	// ACCESSOR
	TranslationRuleHiero* GetUnknownRule(WordId unknown_word) const;
	TranslationRuleHiero* GetGlueRule() { return glue_rule; }
	int GetSpanLimit() { return span_length; } 

	// MUTATOR
	void SetSpanLimit(int length) { span_length = length; }

protected:
	LookupNodeFSM* root_node;
	TranslationRuleHiero* glue_rule;
 	int span_length;
private:
	void AddGlueRule(int start, int end, HyperGraph* ret, std::map<std::pair<int,int>, HyperNode*>* node_map, 
			std::vector<std::pair<int,int> >* span_temp, std::set<GenericString<WordId> >* edge_set) const;
	EdgeKey TransformSpanToKey(const int xbegin, const int xend, const std::vector<std::pair<int,int> > & tail_spans) const;

	HyperGraph * BuildHyperGraph(HyperGraph* ret, NodeMap & node_map, EdgeSet & edge_set,
		const Sentence & input, LookupNodeFSM* node, int position, int last_scan, HieroRuleSpans & spans, bool only_x) const;

	HieroRuleSpans* GetSpanCopy(const LookupTableFSM::HieroRuleSpans spans) const;
	HyperNode* FindNode(map<pair<int,int>, HyperNode*>* map_ptr, const int span_begin, const int span_end) const;
	void AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule);

	HyperEdge* TransformRuleIntoEdge(NodeMap* map, const int head_first, 
			const int head_second, const std::vector<std::pair<int,int> > & tail_spans, TranslationRuleHiero* rule) const;

	HyperEdge* TransformRuleIntoEdge(TranslationRuleHiero* rule, const HieroRuleSpans & rule_span, NodeMap & node_map, EdgeSet & edge_set) const;

	bool NTInSpanLimit(TranslationRuleHiero* rule, const HieroRuleSpans & spans) const;
};



}

#endif