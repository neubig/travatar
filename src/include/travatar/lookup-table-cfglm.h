#ifndef LOOKUP_TABLE_CFGLM_H__
#define LOOKUP_TABLE_CFGLM_H__

#include <travatar/graph-transformer.h>
#include <travatar/rule-fsm.h>
#include <marisa/marisa.h>

namespace travatar {

class LMData;
class CFGPath;

class CFGChartItem {

public:
    CFGChartItem() { }
    ~CFGChartItem() { }

    std::vector<HieroHeadLabels> & GetSyms() { return syms_; }

protected:

    std::vector<HieroHeadLabels> syms_;

};

class CFGCollection {

public:
    CFGCollection() { }
    ~CFGCollection() { }

    void AddRules(const CFGPath & path, const RuleVec & rules) {
        for(size_t i = 0; i < rules.size(); i++)
          rules_.push_back(rules[i]);
    }

protected:

    RuleVec rules_;

};

class CFGPath {

public:
    CFGPath() { }
    CFGPath(const Sentence & sent, int id) : str((char*)&sent[id], sizeof(WordId)), spans(1, std::make_pair(id, id+1)) {
        agent.set_query(str.c_str());
    }
    CFGPath(CFGPath & prev_path, const Sentence & sent, int id) : str(prev_path.str), spans(prev_path.spans) {
        str.append((char*)&sent[id], sizeof(WordId));
        spans.push_back(std::make_pair(id, id+1));
        agent.set_query(str.c_str());
    }
    CFGPath(CFGPath & prev_path, const HieroHeadLabels & heads, int i, int j) : str(prev_path.str), spans(prev_path.spans) {
        Sentence inv_heads(heads.size());
        for(size_t i = 0; i < heads.size(); i++) inv_heads[i] = -1 - heads[i];
        str.append((char*)&inv_heads[0], inv_heads.size()*sizeof(WordId));
        spans.push_back(std::make_pair(i, j));
        agent.set_query(str.c_str());
    }

    std::string str;
    marisa::Agent agent;
    HieroRuleSpans spans;

};

// A lookup table that can simultaneously perform LM integration and
// cube pruning
class LookupTableCFGLM : public GraphTransformer {
protected:

public:

    // Typedefs
    typedef std::vector<TranslationRuleHiero*> RuleVec;
    typedef std::vector<RuleVec> RuleSet; 

    // Constructors
    LookupTableCFGLM();
    ~LookupTableCFGLM();

    // Transform a graph of words into a hiero graph
    virtual HyperGraph * TransformGraph(const HyperGraph & graph) const;

    static LookupTableCFGLM * ReadFromFiles(const std::vector<std::string> & filename);

    // Accessors
    void LoadLM(const std::string & filename, int pop_limit);
    void SetTrgFactors(int trg_factors) { trg_factors_ = trg_factors; }
    void SetRootSymbol(WordId symbol) { root_symbol_ = HieroHeadLabels(std::vector<WordId>(trg_factors_+1,symbol)); }
    void SetUnkSymbol(WordId symbol) { unk_symbol_ = HieroHeadLabels(std::vector<WordId>(trg_factors_+1,symbol)); }
    void AddRuleFSM(RuleFSM* fsm) { rule_fsms_.push_back(fsm); }

private:

    std::vector<RuleFSM*> rule_fsms_;
    LMData* lm_;

    int pop_limit_;
    int trg_factors_;
    HieroHeadLabels root_symbol_;
    HieroHeadLabels unk_symbol_;
    HieroHeadLabels empty_symbol_;

    void Consume(CFGPath & a, const Sentence & sent, int N, int i, int j, int k, std::vector<CFGChartItem> & chart, std::vector<CFGCollection> & collections) const;
    void AddToChart(CFGPath & a, const Sentence & sent, int N, int i, int j, bool u, std::vector<CFGChartItem> & chart, std::vector<CFGCollection> & collections) const;
    void CubePrune(const CFGCollection & collection) const;

};

}

#endif
