#ifndef RULE_FSM_H__
#define RULE_FSM_H__

#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <marisa/marisa.h>
#include <vector>
#include <map>
#include <set>

namespace travatar {

class TranslationRuleHiero;
class HyperNode;
class HyperEdge;
class CfgData;

typedef std::vector<WordId> HieroHeadLabels;
typedef std::vector<std::pair<int,int> > HieroRuleSpans;
typedef std::map<HieroHeadLabels,std::vector<HieroHeadLabels> > UnaryMap;
typedef std::map<HieroHeadLabels, HyperNode*> HeadNodePairs;
typedef std::map<std::pair<int,int>, HeadNodePairs> HieroNodeMap;
typedef std::vector<HyperEdge* > EdgeList;
typedef std::pair<int, std::pair<int,int> > TailSpanKey;
typedef std::vector<TranslationRuleHiero*> RuleVec;
typedef std::vector<RuleVec> RuleSet; 

class RuleFSM {
protected:

    // The trie indexing the rules, and the rules
    marisa::Trie trie_;
    RuleSet rules_;

    // Other statistics
    UnaryMap unaries_;
    int span_length_;
    bool save_src_str_;
public:

    friend class LookupTableFSM;

    RuleFSM() : span_length_(20), save_src_str_(false) { }

    virtual ~RuleFSM();
    
    static RuleFSM * ReadFromRuleTable(std::istream & in);

    static TranslationRuleHiero * BuildRule(travatar::TranslationRuleHiero * rule, std::vector<std::string> & source, 
            std::vector<std::string> & target, SparseMap& features);

    static HyperEdge* TransformRuleIntoEdge(HieroNodeMap& map, const int head_first, 
            const int head_second, const std::vector<TailSpanKey > & tail_spans, TranslationRuleHiero* rule, bool save_src_str=false);

    static HyperEdge* TransformRuleIntoEdge(TranslationRuleHiero* rule, const HieroRuleSpans & rule_span, HieroNodeMap & node_map, bool save_src_str=false);

    static HyperNode* FindNode(HieroNodeMap& map, const int span_begin, const int span_end, const HieroHeadLabels& head_label);

    // ACCESSOR
    int GetSpanLimit() const { return span_length_; } 
    const RuleSet & GetRules() const { return rules_; }
    const UnaryMap & GetUnaryMap() const { return unaries_; }
    const marisa::Trie & GetTrie() const { return trie_; }
    RuleSet & GetRules() { return rules_; }
    marisa::Trie & GetTrie() { return trie_; }
 
    // MUTATOR
    void SetSpanLimit(const int length) { span_length_ = length; }
    void SetSaveSrcStr(const bool save_src_str) { save_src_str_ = save_src_str; }

protected:
    void BuildHyperGraphComponent(HieroNodeMap & node_map, EdgeList & edge_set,
        const Sentence & input, const std::string & state, int position, HieroRuleSpans & spans) const;

    static std::string CreateKey(const CfgData & src_data,
                                 const std::vector<CfgData> & trg_data);
private:
    // void AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule);
};

}

// inline std::ostream &operator<<( std::ostream &out, const RuleFSM &L ) {
//     L.Print(out);
//     return out;
// }

#endif
