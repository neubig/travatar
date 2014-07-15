#ifndef LOOKUP_TABLE_H__
#define LOOKUP_TABLE_H__

#include <travatar/graph-transformer.h>
#include <travatar/sparse-map.h>
#include <travatar/translation-rule.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <climits>

namespace travatar {

class HyperNode;

// A single state for a partial rule match
// This must be overloaded with a state that is used in a specific implementation
class LookupState {
public:
    LookupState() { }
    virtual ~LookupState() { }

    const std::vector<const HyperNode*> & GetNonterms() const { return nonterm_nodes_; }
    std::vector<const HyperNode*> & GetNonterms() { return nonterm_nodes_; }
    void SetNonterms(const std::vector<const HyperNode*> & nonterm_nodes) { nonterm_nodes_ = nonterm_nodes; }
    const SparseMap & GetFeatures() const { return features_; }
    void SetFeatures(const SparseMap & feat) { features_ = feat; }
    void AddFeatures(const SparseMap & feat) { features_ += feat; }

protected:
    // Links to the nodes of non-terminals that are abstracted
    std::vector<const HyperNode*> nonterm_nodes_;
    SparseMap features_;
};

// An implementation of a GraphTransformer that takes as input
// a parse forest of the original sentence, looks up rules, and
// outputs a rule graph in the target language
class LookupTable : public GraphTransformer {
public:
    LookupTable();
    virtual ~LookupTable();

    virtual HyperGraph * TransformGraph(const HyperGraph & parse) const;

    // Find all the translation rules rooted at a particular node in a parse graph
    std::vector<boost::shared_ptr<LookupState> > LookupSrc(
            const HyperNode & node, 
            const std::vector<boost::shared_ptr<LookupState> > & old_states) const;

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const = 0;

    // Get the unknown rule
    const TranslationRule * GetUnknownRule() const;

    virtual LookupState * GetInitialState() const = 0;

    // Match all unknown words or not
    void SetMatchAllUnk(bool match_all_unk) { match_all_unk_ = match_all_unk; }
    bool GetMatchAllUnk() { return match_all_unk_; }

protected:

    // Match a single node
    // For example S(NP(PRN("he")) x0:VP) will match for "he" and VP
    // If matching a non-terminal (e.g. VP), advance the state and push "node"
    // on to the list of non-terminals. Otherwise, just advance the state
    // Returns NULL if no rules were matched
    virtual LookupState * MatchNode(const HyperNode & node, const LookupState & state) const = 0;

    // Match the start of an edge
    // For example S(NP(PRN("he")) x0:VP) will match the opening bracket 
    // of S( or NP( or PRN(
    virtual LookupState * MatchStart(const HyperNode & node, const LookupState & state) const = 0;
    
    // Match the end of an edge
    // For example S(NP(PRN("he")) x0:VP) will match the closing brackets for
    // of (S...) or (NP...) or (PRN...)
    virtual LookupState * MatchEnd(const HyperNode & node, const LookupState & state) const = 0;

    TranslationRule unk_rule_;
    // Match all nodes with the unknown rule, not just when no other rule is matched
    bool match_all_unk_;

};

}

#endif
