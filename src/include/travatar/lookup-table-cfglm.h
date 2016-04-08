#ifndef LOOKUP_TABLE_CFGLM_H__
#define LOOKUP_TABLE_CFGLM_H__

#include <travatar/graph-transformer.h>
#include <travatar/rule-fsm.h>
#include <marisa/marisa.h>

namespace lm { namespace ngram { struct ChartState; } }

namespace travatar {

class LMFunc;
class LMData;
class CFGPath;
class Weights;

class CFGChartItem {

public:
    typedef std::pair<HyperNode*, std::vector<lm::ngram::ChartState> > StatefulNode;
    typedef boost::unordered_map<HieroHeadLabels, std::vector<StatefulNode*> > StatefulNodeMap;

    CFGChartItem() : populated_(false) { }
    ~CFGChartItem();

    StatefulNodeMap & GetNodes() { return nodes_; }
    bool IsPopulated() { return populated_; }

    Real GetHypScore(const HieroHeadLabels & label, int pos) const;
    Real GetHypScoreDiff(const HieroHeadLabels & label, int pos) const {
        return GetHypScore(label, pos) - GetHypScore(label, pos-1);
    }

    const StatefulNode & GetStatefulNode(const HieroHeadLabels & label, int pos) const;
    void AddStatefulNode(const HieroHeadLabels & label, HyperNode* node, const std::vector<lm::ngram::ChartState> & state);
    void FinalizeNodes();

protected:

    bool populated_;
    StatefulNodeMap nodes_;

};

class CFGPath {

public:
    CFGPath() { }
    CFGPath(const Sentence & sent, int id) : str((char*)&sent[id], sizeof(WordId)) {
        agent.set_query(str.c_str(), str.length());
    }
    CFGPath(CFGPath & prev_path, const Sentence & sent, int id) : str(prev_path.str), labels(prev_path.labels), spans(prev_path.spans) {
        assert(id < sent.size());
        str.append((char*)&sent[id], sizeof(WordId));
        agent.set_query(str.c_str(), str.length());
        assert(labels.size() == spans.size());
    }
    CFGPath(CFGPath & prev_path, const HieroHeadLabels & heads, int i, int j) : str(prev_path.str), labels(prev_path.labels), spans(prev_path.spans) {
        Sentence inv_heads(heads.size());
        for(size_t i = 0; i < heads.size(); i++) inv_heads[i] = -1 - heads[i];
        str.append((char*)&inv_heads[0], inv_heads.size()*sizeof(WordId));
        labels.push_back(heads);
        spans.push_back(std::make_pair(i, j));
        assert(labels.size() == spans.size());
        agent.set_query(str.c_str(), str.length());
    }

    static std::string PrintAgent(const marisa::Agent & agent) {
        if(agent.query().length() == 0) return "";
        std::ostringstream oss;
        const char* ptr = agent.query().ptr();
        for(size_t i = 0; i < agent.query().length(); i += sizeof(WordId)) {
            oss << (i==0 ? "" : " ") << *(WordId*)(ptr+i);
        }
        return oss.str();
    }
    static std::string PrintAgentKey(const marisa::Agent & agent) {
        if(agent.key().length() == 0) return "";
        std::ostringstream oss;
        const char* ptr = agent.key().ptr();
        for(size_t i = 0; i < agent.key().length(); i += sizeof(WordId)) {
            oss << (i==0 ? "" : " ") << *(WordId*)(ptr+i);
        }
        return oss.str();
    }

    std::string str;
    marisa::Agent agent;
    std::vector<HieroHeadLabels> labels;
    HieroRuleSpans spans;

};

class CFGCollection {

public:
    typedef std::vector<std::shared_ptr<HieroRuleSpans> > SpanVec;
    typedef std::vector<std::shared_ptr<std::vector<HieroHeadLabels> > > LabelVec;

    CFGCollection() { }
    ~CFGCollection() { }

    void AddRules(const CFGPath & path, const RuleVec & rules) {
        std::shared_ptr<HieroRuleSpans> span(new HieroRuleSpans(path.spans));
        std::shared_ptr<std::vector<HieroHeadLabels> > label(new std::vector<HieroHeadLabels>(path.labels));
        for(size_t i = 0; i < rules.size(); i++) {
            rules_.push_back(rules[i]);
            spans_.push_back(span);
            labels_.push_back(label);
        }
    }

    const RuleVec & GetRules() const { return rules_; }
    const SpanVec & GetSpans() const { return spans_; }
    const LabelVec & GetLabels() const { return labels_; }

protected:

    RuleVec rules_;
    SpanVec spans_;
    LabelVec labels_;

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
    bool PredictiveSearch(marisa::Agent & agent) const;

    static LookupTableCFGLM * ReadFromFiles(const std::vector<std::string> & filename);

    // Accessors
    void LoadLM(const std::string & filename);
    void SetPopLimit(int pop_limit) { pop_limit_ = pop_limit; }
    void SetChartLimit(int chart_limit) { chart_limit_ = chart_limit; }
    void SetTrgFactors(int trg_factors) { trg_factors_ = trg_factors; }
    void SetWeights(const Weights & weights) { weights_ = &weights; }
    void SetRootSymbol(WordId symbol) { root_symbol_ = HieroHeadLabels(std::vector<WordId>(trg_factors_+1,symbol)); }
    void SetUnkSymbol(WordId symbol) { unk_symbol_ = HieroHeadLabels(std::vector<WordId>(trg_factors_+1,symbol)); }
    void AddRuleFSM(RuleFSM* fsm) { rule_fsms_.push_back(fsm); }

private:

    std::vector<RuleFSM*> rule_fsms_;
    std::vector<LMData*> lm_data_;
    std::vector<LMFunc*> funcs_;
    int pop_limit_;
    int chart_limit_;
    
    int trg_factors_;
    HieroHeadLabels root_symbol_;
    HieroHeadLabels unk_symbol_;
    HieroHeadLabels empty_symbol_;
    const Weights * weights_;

    void Consume(CFGPath & a, const Sentence & sent, int N, int i, int j, int k, std::vector<CFGChartItem> & chart, std::vector<CFGCollection> & collections) const;
    void AddToChart(CFGPath & a, const Sentence & sent, int N, int i, int j, bool u, std::vector<CFGChartItem> & chart, std::vector<CFGCollection> & collections) const;
    void CubePrune(int N, int i, int j, std::vector<CFGCollection> & collection, std::vector<CFGChartItem> & chart, HyperGraph & ret) const;

};

}

#endif
