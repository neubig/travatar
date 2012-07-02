#ifndef LOOKUP_TABLE_H__
#define LOOKUP_TABLE_H__

#include <vector>
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
    LookupTable() { }
    virtual ~LookupTable() { };

    // // Build the rule hypergraph
    // HyperGraph * BuildRuleHypergraph(const HyperGraph & parse) {
    //     // Create the hyper-graph to return, and add its nodes
    //     HyperGraph * ret = new HyperGraph;
    //     BOOST_FOREACH(HyperNode * node, parse.GetNodes()) {
    //         HyperNode* next_node = new HyperNode;
    //         next_node.SetSym(node->GetSym());
    //         next_node.SetSpan(node->GetSpan());
    //         ret->GetNodes().push_back(next_node);
    //     }
    //     // Get the initial state
    //     std::vector<boost::shared_ptr<LookupState> > initial(1);
    //     initial[0].reset(GetInitialState());
    //     // Find the rule matches for each node
    //     BOOST_FOREACH(HyperNode * node, parse.GetNodes()) {
    //         std::vector<boost::shared_ptr<LookupState> > srcs = LookupSrc(*node, initial);
    //         if(srcs.size() > 0) {
    //             BOOST_FOREACH(const boost::shared_ptr<LookupState> & src, srcs) {
    //                 HyperEdge* base_edge = new HyperEdge;
    //                 // Add the lookup's connecting nodes, if any
    //                 HERE
    //                 BOOST_FOREACH(const TranslationRule * rule, SafeReference(FindRules(*src))) {
    //                     
    //                 }
    //             }
    //         } else if (node->IsTerminal()) {
    //             // TODO
    //         } else {
    //             // TODO
    //         }
    //     }
    // }

    // Find all the translation rules rooted at a particular node in a parse graph
    std::vector<boost::shared_ptr<LookupState> > LookupSrc(
            const HyperNode & node, 
            const std::vector<boost::shared_ptr<LookupState> > & old_states);

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const = 0;

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

};

}

#endif
