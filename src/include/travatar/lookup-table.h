#ifndef LOOKUP_TABLE_H__
#define LOOKUP_TABLE_H__

#include <vector>
#include <map>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>
#include <boost/foreach.hpp>

namespace travatar {

// A single state for a partial rule match
// This must be overloaded with a state that is used in a specific implementation
class LookupState {
public:
    LookupState() { }
    virtual ~LookupState() { }

    const std::vector<const HyperNode*> & GetNonterms() const { return nonterm_nodes_; }
    std::vector<const HyperNode*> & GetNonterms() { return nonterm_nodes_; }
    void SetNonterms(const std::vector<const HyperNode*> & nonterm_nodes) { nonterm_nodes_ = nonterm_nodes; }

protected:
    // Links to the nodes of non-terminals that are abstracted
    std::vector<const HyperNode*> nonterm_nodes_;
};

// A table that allows rules to be looked up
// This will be overloaded with an actual implementation
class LookupTable {
public:
    LookupTable() : unk_rule_("UNK", Dict::ParseQuotedWords("x0"), Dict::ParseFeatures("unk=1")) { }
    virtual ~LookupTable() { };

    HyperGraph * BuildRuleGraph(const HyperGraph & parse);

    // Find all the translation rules rooted at a particular node in a parse graph
    std::vector<boost::shared_ptr<LookupState> > LookupSrc(
            const HyperNode & node, 
            const std::vector<boost::shared_ptr<LookupState> > & old_states);

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const = 0;

    // Get the unknown rule
    const TranslationRule * GetUnknownRule() const { return &unk_rule_; }

    virtual LookupState * GetInitialState() = 0;

protected:

    // Match a single node
    // For example S(NP(PRN("he")) x0:VP) will match for "he" and VP
    // If matching a non-terminal (e.g. VP), advance the state and push "node"
    // on to the list of non-terminals. Otherwise, just advance the state
    // Returns NULL if no rules were matched
    virtual LookupState * MatchNode(const HyperNode & node, const LookupState & state) = 0;

    // Match the start of an edge
    // For example S(NP(PRN("he")) x0:VP) will match the opening bracket 
    // of S( or NP( or PRN(
    virtual LookupState * MatchStart(const HyperNode & node, const LookupState & state) = 0;
    
    // Match the end of an edge
    // For example S(NP(PRN("he")) x0:VP) will match the closing brackets for
    // of (S...) or (NP...) or (PRN...)
    virtual LookupState * MatchEnd(const HyperNode & node, const LookupState & state) = 0;

    TranslationRule unk_rule_;

};

}

#endif
