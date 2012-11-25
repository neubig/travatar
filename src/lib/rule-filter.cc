#include <travatar/rule-filter.h>
#include <travatar/hyper-graph.h>
#include <travatar/util.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;

bool RuleSizeFilter::PassesFilter(
            const HyperEdge & rule,
            const Sentence & src_sent,
            const Sentence & trg_sent) const {
    if(rule.NumTails() > nonterm_len_) {
        PRINT_DEBUG("RuleSizeFilter: " << rule << ": nonterm" << endl, 2);
        return false;
    }
    std::pair<int,int> src_cov = rule.GetHead()->GetSpan(), 
                       trg_cov = rule.GetHead()->GetTrgCovered();
    int src_len = src_cov.second - src_cov.first,
        trg_len = min(trg_cov.second,(int)trg_sent.size()) - trg_cov.first;
    BOOST_FOREACH(const HyperNode * tail, rule.GetTails()) {
        std::pair<int,int> tsrc_cov = tail->GetSpan(), ttrg_cov = tail->GetTrgCovered();
        src_len -= tsrc_cov.second - tsrc_cov.first;
        trg_len -= min(ttrg_cov.second,(int)trg_sent.size()) - ttrg_cov.first;            
    }
    bool passes = (src_len <= term_len_) && (trg_len <= term_len_);
    PRINT_DEBUG("RuleSizeFilter: " << rule << ": " << passes << " (("<<src_len<<" <= "<<term_len_<<") && ("<<trg_len<<" <= "<<term_len_<<") "<<endl, 2);
    return passes;
}

bool CountFilter::PassesFilter(
    const HyperEdge & rule,
    const Sentence & src_sent,
    const Sentence & trg_sent) const {
    bool passes = rule.GetScore() > score_thresh_;
    PRINT_DEBUG("CountFilter: " << rule << ": " << passes << endl, 2);
    return passes;
}

bool PseudoNodeFilter::PassesFilter(
    const HyperEdge & rule,
    const Sentence & src_sent,
    const Sentence & trg_sent) const {
    bool not_pseudo = rule.GetHead()->IsFrontier() != HyperNode::NOT_FRONTIER;
    PRINT_DEBUG("PseudoFilter: " << rule << ": " << not_pseudo << endl, 2);
    return not_pseudo;    
}
