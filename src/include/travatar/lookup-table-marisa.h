#ifndef LOOKUP_TABLE_MARISA_H__
#define LOOKUP_TABLE_MARISA_H__

#include <vector>
#include <travatar/hyper-graph.h>
#include <travatar/lookup-table.h>
#include <marisa.h>
#include <boost/foreach.hpp>


namespace travatar {

// A single state for a partial rule match
class LookupStateMarisa : public LookupState {
public:
    LookupStateMarisa() : LookupState() { }
    virtual ~LookupStateMarisa() { }

    const std::string & GetString() const { return curr_string_; }
    void SetString(const std::string & str) { curr_string_ = str; }
protected:
    // A string representing the current progress
    std::string curr_string_;
};

// A table that allows rules to be looked up in a hash table
class LookupTableMarisa : public LookupTable {
public:
    LookupTableMarisa() { }
    virtual ~LookupTableMarisa() {
        BOOST_FOREACH(std::vector<TranslationRule*> & vec, rules_)
            BOOST_FOREACH(TranslationRule * rule, vec)
                delete rule;
    };

    virtual LookupState * GetInitialState() {
        return new LookupStateMarisa;
    }

    static LookupTableMarisa * ReadFromRuleTable(std::istream & in);

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const;

protected:

    // Match a single node
    virtual LookupState * MatchNode(const HyperNode & node, const LookupState & state);

    // Match the start of an edge
    virtual LookupState * MatchStart(const HyperNode & node, const LookupState & state) {
        const std::string & p = ((const LookupStateMarisa &)state).GetString();
        std::string next = p + (p.size()?" ":"") + Dict::WSym(node.GetSym()) + " (";
        return MatchState(next, state);
    }
    
    // Match the end of an edge
    virtual LookupState * MatchEnd(const HyperNode & node, const LookupState & state) {
        std::string next = ((const LookupStateMarisa &)state).GetString() + " )";
        return MatchState(next, state);
    }

    LookupStateMarisa * MatchState(const std::string & next, const LookupState & state);

    // void AddRule(TranslationRule * rule) {
    //     rules_[rule->GetSrcStr()].push_back(rule);
    // }

   typedef std::vector< std::vector<TranslationRule*> > RuleSet; 
   const RuleSet & GetRules() const { return rules_; }
   RuleSet & GetRules() { return rules_; }
   const marisa::Trie & GetTrie() const { return trie_; }
   marisa::Trie & GetTrie() { return trie_; }

protected:
    marisa::Trie trie_;
    RuleSet rules_;

};

}

#endif
