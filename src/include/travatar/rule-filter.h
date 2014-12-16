#ifndef RULE_FILTER_H__
#define RULE_FILTER_H__

#include <travatar/sentence.h>
#include <travatar/real.h>
#include <cmath>
#include <cfloat>

namespace travatar {

class HyperEdge;

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
        const Sentence & trg_sent) const;
};

// Don't return rules headed by pseudo-nodes
class CountFilter : public RuleFilter {
public:
    
    CountFilter(Real prob_thresh) : score_thresh_(prob_thresh ? log(prob_thresh) : -REAL_MAX) { }
    virtual ~CountFilter() { };

    virtual bool PassesFilter(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent) const;
private:
    Real score_thresh_;
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
                const Sentence & trg_sent) const;
    
};

}

#endif
