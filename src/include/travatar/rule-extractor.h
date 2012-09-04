#ifndef TRAVATAR_RULE_EXTRACTOR__
#define TRAVATAR_RULE_EXTRACTOR__

#include <boost/shared_ptr.hpp>
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <set>
#include <list>

namespace travatar {

// A virtual class to overload that converts a source parse, a target sentence
// and an alignment into a forest of matched rules
class RuleExtractor {
public:

    // Create the minimal graph of rules
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const = 0;

    // Take an edge that represents a rule and the corresponding target sentence
    // and return a string version of the rule
    std::string RuleToString(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent) const;

private:
    // A function to help print rules recursively
    void PrintRuleSurface(const HyperNode & node,
                          const Sentence & src_sent,
                          std::list<HyperEdge*> & remaining_fragments,
                          int & tail_num,
                          std::ostream & oss) const;

};

// A rule extractor using forest based rule extraction
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
class ForestExtractor : public RuleExtractor{
public:

    ForestExtractor() : max_attach_(1) { }

    // Extract the very minimal set of rules (no nulls, etc)
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const;

    // Attach null-aligned target words to the very top node
    // that they can be attached to
    HyperGraph * AttachNullsTop(const HyperGraph & rule_graph,
                                const Alignment & align,
                                int trg_len);

    // Attach null-aligned target words to all possible nodes that
    // they can be attached to
    HyperGraph * AttachNullsExhaustive(const HyperGraph & rule_graph,
                                       const Alignment & align,
                                       int trg_len);

    // For expanding all nulls exhaustively
    typedef std::vector<std::pair<std::set<int>, HyperNode*> > SpanNodeVector;

    // Used memoized recursion to get the expanded nodes for each old node
    const SpanNodeVector & GetExpandedNodes(
                            const std::vector<bool> & nulls,
                            const HyperNode & old_node,
                            std::vector<SpanNodeVector> & expanded);

    SpanNodeVector ExpandNode(
                const std::vector<bool> & nulls,
                const HyperNode & old_node) const;

    void SetMaxAttach(int max_attach) {
        max_attach_ = max_attach;
    }

protected:

    int max_attach_;

    void AttachNullsTop(std::vector<bool> & nulls,
                        HyperNode & node);
    void AttachNullsExhaustive(std::vector<bool> & nulls,
                               HyperNode & node);

};

}

#endif
