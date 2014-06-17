#ifndef TRAVATAR_FOREST_EXTRACTOR__
#define TRAVATAR_FOREST_EXTRACTOR__

#include <travatar/rule-extractor.h>

namespace travatar {

// A rule extractor using forest based rule extraction
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
class ForestExtractor : public RuleExtractor{
public:

    ForestExtractor() : max_attach_(1), max_nonterm_(5) { }

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
                            std::vector<SpanNodeVector> & expanded,
                            int my_attach);

    SpanNodeVector ExpandNode(
                const std::vector<bool> & nulls,
                const HyperNode & old_node,
                int my_attach) const;

    void SetMaxAttach(int max_attach) { max_attach_ = max_attach; }
    void SetMaxNonterm(int max_nonterm) { max_nonterm_ = max_nonterm; }

protected:

    int max_attach_;
    int max_nonterm_;

    void AttachNullsTop(std::vector<bool> & nulls,
                        HyperNode & node);
    void AttachNullsExhaustive(std::vector<bool> & nulls,
                               HyperNode & node);

};

}

#endif
