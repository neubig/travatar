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

namespace travatar {
class LookupNodeFSM {
typedef std::map<WordId, LookupNodeFSM*> HieroNodeMap;
protected:
    HieroNodeMap lookup_map;
    std::vector<TranslationRuleHiero*> rules;
    std::set<WordId> labels;
public:
    LookupNodeFSM() { }

    virtual ~LookupNodeFSM() { 
        BOOST_FOREACH(HieroNodeMap::value_type &it, lookup_map) {
            delete it.second++;
        }
        BOOST_FOREACH(TranslationRuleHiero* rule, rules) {
            delete rule;
        }
    }
    
    virtual void AddEntry(WordId & key, LookupNodeFSM* rule);
    virtual LookupNodeFSM* FindNode(WordId key) const;
    virtual void AddRule(TranslationRuleHiero* rule);
    virtual std::string ToString(int indent) const;
    virtual std::vector<TranslationRuleHiero*> & GetTranslationRules() { return rules; }
};  

typedef std::vector<std::pair<int,int> > HieroRuleSpans;
typedef std::pair<WordId, std::pair<int,int> > HieroNodeKey;
typedef std::map<HieroNodeKey, HyperNode*> HieroNodeMap;
typedef std::vector<HyperEdge* > EdgeList;
typedef std::pair<int, std::pair<int,int> > TailSpanKey;

class RuleFSM {
protected:
    LookupNodeFSM* root_node_;
    int span_length_;
public:

    friend class LookupTableFSM;

    RuleFSM() : root_node_(new LookupNodeFSM), span_length_(20) { }

    virtual ~RuleFSM() { 
        delete root_node_;
    }
    
    static RuleFSM * ReadFromRuleTable(std::istream & in);

    static TranslationRuleHiero * BuildRule(travatar::TranslationRuleHiero * rule, std::vector<std::string> & source, 
            std::vector<std::string> & target, SparseMap features);

    virtual void AddRule(TranslationRuleHiero* rule);

    // ACCESSOR
    int GetSpanLimit() const { return span_length_; } 
    LookupNodeFSM* GetRootNode() const { return root_node_; }
 
    // MUTATOR
    void SetSpanLimit(int length) { span_length_ = length; }
    
    // ToString
    std::string ToString() const;
protected:
    void BuildHyperGraphComponent(HieroNodeMap & node_map, EdgeList & edge_set,
        const Sentence & input, LookupNodeFSM* node, int position, HieroRuleSpans & spans) const;

private:
    void AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule);
};

class LookupTableFSM : public GraphTransformer {
protected:
    std::vector<RuleFSM*> rule_fsms_;
    bool delete_unknown_;
    WordId default_symbol_;
    WordId root_symbol_;

public:
    LookupTableFSM() : rule_fsms_(),
                       delete_unknown_(false),
                       default_symbol_(Dict::WID("X")),
                       root_symbol_(Dict::WID("S")) { }
    ~LookupTableFSM() {
        BOOST_FOREACH(RuleFSM* rule_fsm, rule_fsms_) {
            if(rule_fsm != NULL)
                delete rule_fsm;
        }
    }

    void AddRuleFSM(RuleFSM* fsm) {
        rule_fsms_.push_back(fsm);
    }

    // Transform a graph of words into a hiero graph
    virtual HyperGraph * TransformGraph(const HyperGraph & graph) const;

    TranslationRuleHiero* GetUnknownRule(WordId unknown_word, WordId symbol) const;
    WordId GetRootSymbol() const { return root_symbol_; } 
    WordId GetDefaultSymbol() const { return default_symbol_; }

    void SetDeleteUnknown(bool delete_unk) { delete_unknown_ = delete_unk; }
    void SetRootSymbol(WordId symbol) { root_symbol_ = symbol; }
    void SetDefaultSymbol(WordId symbol) { default_symbol_ = symbol; }
    void SetSpanLimits(const std::vector<int> limits);

    static LookupTableFSM * ReadFromFiles(const std::vector<std::string> & filenames);

    static HyperEdge* TransformRuleIntoEdge(HieroNodeMap* map, const int head_first, 
            const int head_second, const std::vector<TailSpanKey > & tail_spans, TranslationRuleHiero* rule);

    static HyperEdge* TransformRuleIntoEdge(TranslationRuleHiero* rule, const HieroRuleSpans & rule_span, HieroNodeMap & node_map);

    static HyperNode* FindNode(HieroNodeMap* map_ptr, const int span_begin, const int span_end, const WordId);

protected:
    void CleanUnreachableNode(EdgeList & edge_list, HieroNodeMap & node_map) const;

};
}

#endif
