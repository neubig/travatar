#ifndef RULE_FILTER_H__
#define RULE_FILTER_H__

#include <travatar/hyper-graph.h>

namespace travatar {

class RuleFilter {

public:
    RuleFilter() { }
    virtual ~RuleFilter() { };

    // This function implements the filter. It should return "true" if the rule is
    // OK or "false" if the rule is bad
    virtual bool PassesFilter(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent) const = 0;

};

// Don't return rules headed by pseudo-nodes
class PseudoNodeFilter : public RuleFilter {
public:
    
    PseudoNodeFilter() { }
    virtual ~PseudoNodeFilter() { };

    virtual bool PassesFilter(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent) const {
        return rule.GetHead()->IsFrontier() != HyperNode::NOT_FRONTIER;
    }
};

// Don't return rules headed by pseudo-nodes
class CountFilter : public RuleFilter {
public:
    
    CountFilter(double prob_thresh) : score_thresh_(prob_thresh ? log(prob_thresh) : -DBL_MAX) { }
    virtual ~CountFilter() { };

    virtual bool PassesFilter(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent) const {
        return rule.GetScore() > score_thresh_;
    }
private:
    double score_thresh_;
};

// Don't return rules greater than a certain length
class RuleSizeFilter : public RuleFilter {
protected:
    int term_len_, nonterm_len_;
public:
    RuleSizeFilter(int term_len, int nonterm_len) :
        term_len_(term_len), nonterm_len_(nonterm_len) { }

    virtual ~RuleSizeFilter() { };
    virtual bool PassesFilter(
                const HyperEdge & rule,
                const Sentence & src_sent,
                const Sentence & trg_sent) const {
        if(rule.NumTails() > nonterm_len_) return false;
        std::pair<int,int> src_cov = rule.GetHead()->GetSpan(), 
                           trg_cov = rule.GetHead()->GetTrgCovered();
        int src_len = src_cov.second - src_cov.first,
            trg_len = trg_cov.second - trg_cov.first;
        BOOST_FOREACH(const HyperNode * tail, rule.GetTails()) {
            std::pair<int,int> tsrc_cov = tail->GetSpan(), ttrg_cov = tail->GetTrgCovered();
            src_len -= tsrc_cov.second - tsrc_cov.first;
            trg_len -= ttrg_cov.second - ttrg_cov.first;            
        }
        return (src_len <= term_len_) && (trg_len <= term_len_);
    }
    
};

}

#endif
