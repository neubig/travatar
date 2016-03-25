#ifndef LOOKUP_TABLE_FSM_H__
#define LOOKUP_TABLE_FSM_H__

#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <travatar/rule-fsm.h>
#include <marisa/marisa.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <set>

namespace travatar {

class HyperNode;
class HyperEdge;
class LookupNodeFSM;
class TranslationRuleHiero;

typedef std::map<WordId, LookupNodeFSM*> LookupNodeMap;
typedef std::map<HieroHeadLabels, LookupNodeFSM*> NTLookupNodeMap;

class LookupNodeFSM {
protected:
    LookupNodeMap lookup_map_;
    NTLookupNodeMap nt_lookup_map_;
    std::vector<TranslationRuleHiero*> rules_;
    std::set<WordId> labels_;
public:
    LookupNodeFSM() { }
    virtual ~LookupNodeFSM();
    
    void AddEntry(const WordId & key, LookupNodeFSM* chile_node);
    void AddNTEntry(const HieroHeadLabels& key, LookupNodeFSM* child_node);
    LookupNodeFSM* FindChildNode(const WordId key) const;
    LookupNodeFSM* FindNTChildNode (const HieroHeadLabels& key) const;
    LookupNodeMap & GetNodeMap() { return lookup_map_; }
    const LookupNodeMap & GetNodeMap() const { return lookup_map_; }
    const NTLookupNodeMap & GetNTNodeMap() const { return nt_lookup_map_; }
    void AddRule(TranslationRuleHiero* rule);
    const std::vector<TranslationRuleHiero*> & GetTranslationRules() const { return rules_; }

    virtual void Print(std::ostream &out, WordId label, int indent, char prefix) const; 
}; 

// inline std::ostream &operator<<( std::ostream &out, const LookupNodeFSM &L ) {
//     L.Print(out,Dict::WID("ROOT"),0,'-');
//     return out;
// }

class LookupTableFSM : public GraphTransformer {
protected:
    std::vector<RuleFSM*> rule_fsms_;
    bool delete_unknown_;
    int trg_factors_;
    HieroHeadLabels root_symbol_;
    HieroHeadLabels unk_symbol_;
    HieroHeadLabels empty_symbol_;
    bool save_src_str_;
public:
    LookupTableFSM();
    ~LookupTableFSM();

    void AddRuleFSM(RuleFSM* fsm) {
        rule_fsms_.push_back(fsm);
    }

    // Transform a graph of words into a hiero graph
    virtual HyperGraph * TransformGraph(const HyperGraph & graph) const;

    const HieroHeadLabels & GetRootSymbol() const { return root_symbol_; } 
    const HieroHeadLabels & GetUnkSymbol() const { return unk_symbol_; } 
    bool GetDeleteUnknown() const { return delete_unknown_; } 

    void SetDeleteUnknown(bool delete_unk) { delete_unknown_ = delete_unk; }
    void SetRootSymbol(WordId symbol) { root_symbol_ = HieroHeadLabels(std::vector<WordId>(trg_factors_+1,symbol)); }
    void SetUnkSymbol(WordId symbol) { unk_symbol_ = HieroHeadLabels(std::vector<WordId>(trg_factors_+1,symbol)); }
    void SetSpanLimits(const std::vector<int>& limits);
    void SetTrgFactors(const int trg_factors) { trg_factors_ = trg_factors; } 
    void SetSaveSrcStr(const bool save_src_str);
    static TranslationRuleHiero* GetUnknownRule(const WordId unknown_word, const HieroHeadLabels& head_labels);
    static TranslationRuleHiero* GetUnknownRule(const WordId src, WordId unknown_word, const HieroHeadLabels& head_labels);

    static LookupTableFSM * ReadFromFiles(const std::vector<std::string> & filenames);

private:
    HyperEdge * LookupUnknownRule(int index, const Sentence & sent, const HieroHeadLabels & syms, HieroNodeMap & node_map) const;
};
}

#endif
