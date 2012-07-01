#ifndef LOOKUP_TABLE_H__
#define LOOKUP_TABLE_H__

#include <vector>
#include <travatar/hyper-graph.h>
#include <boost/foreach.hpp>

namespace travatar {

// A single state for a partial rule match
// This must be overloaded with a state that is used in a specific implementation
class LookupState {
public:
    LookupState() { }
    virtual ~LookupState() { }
protected:
    // Links to the nodes of non-terminals that are abstracted
    std::vector<HyperNode*> nonterm_nodes_;
};

// A table that allows rules to be looked up
// This will be overloaded with an actual implementation
class LookupTable {
public:
    LookupTable() { }
    virtual ~LookupTable() { };

    // Find all the translation rules rooted at a particular node in a parse graph
    std::vector<boost::shared_ptr<LookupState> > LookupTranslationRules(
            const HyperNode & node, 
            const std::vector<boost::shared_ptr<LookupState> > & old_states);

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

};

}

#endif
