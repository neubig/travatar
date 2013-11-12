#ifndef LM_COMPOSER_BU_H__
#define LM_COMPOSER_BU_H__

#include <string>
#include <boost/shared_ptr.hpp>
#include <lm/left.hh>
#include <travatar/lm-composer.h>

namespace travatar {

class HyperNode;
class HyperGraph;

typedef std::vector<HyperNode*> ChartEntry;

// A bottom up language model composer that uses cube pruning to keep the
// search space small
class LMComposerBU : public LMComposer {

protected:

    // The maximum number of stack items popped during search
    int stack_pop_limit_;
    // The maximum number of elements in a single chart cell
    int chart_limit_;

public:
    LMComposerBU(lm::ngram::Model * lm) :
        LMComposer(lm), stack_pop_limit_(0), chart_limit_(0) { }
    virtual ~LMComposerBU() { }

    // Intersect this graph with a language model, using cube pruning to control
    // the overall state space.
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const;

    int GetStackPopLimit() const { return stack_pop_limit_; }
    void SetStackPopLimit(double stack_pop_limit) { stack_pop_limit_ = stack_pop_limit; }
    int GetChartLimit() const { return chart_limit_; }
    void SetChartLimit(double chart_limit) { chart_limit_ = chart_limit; }

protected:

    // Build a chart entry for one of the nodes in the input parse
    const ChartEntry & BuildChart(
                        const HyperGraph & parse,
                        std::vector<boost::shared_ptr<ChartEntry> > & chart, 
                        std::vector<lm::ngram::ChartState> & states, 
                        int id,
                        HyperGraph & graph) const;

};

}

#endif
