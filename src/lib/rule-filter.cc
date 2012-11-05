#include <travatar/rule-filter.h>
#include <travatar/hyper-graph.h>
#include <boost/foreach.hpp>

using namespace travatar;

bool RuleSizeFilter::PassesFilter(
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

bool CountFilter::PassesFilter(
    const HyperEdge & rule,
    const Sentence & src_sent,
    const Sentence & trg_sent) const {
    return rule.GetScore() > score_thresh_;
}

bool PseudoNodeFilter::PassesFilter(
    const HyperEdge & rule,
    const Sentence & src_sent,
    const Sentence & trg_sent) const {
    return rule.GetHead()->IsFrontier() != HyperNode::NOT_FRONTIER;
}
