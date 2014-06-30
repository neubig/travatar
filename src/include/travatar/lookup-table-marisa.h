#ifndef LOOKUP_TABLE_MARISA_H__
#define LOOKUP_TABLE_MARISA_H__

#include <vector>
#include <travatar/lookup-table.h>
#include <marisa/marisa.h>

namespace travatar {

class HyperNode;

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
    virtual ~LookupTableMarisa();

    virtual LookupState * GetInitialState() const {
        return new LookupStateMarisa;
    }

    static LookupTableMarisa * ReadFromFile(std::string & filename);
    static LookupTableMarisa * ReadFromRuleTable(std::istream & in);

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const;

protected:

    // Match a single node
    virtual LookupState * MatchNode(const HyperNode & node, const LookupState & state) const;

    // Match the start of an edge
    virtual LookupState * MatchStart(const HyperNode & node, const LookupState & state) const;
    
    // Match the end of an edge
    virtual LookupState * MatchEnd(const HyperNode & node, const LookupState & state) const;

    LookupStateMarisa * MatchState(const std::string & next, const LookupState & state) const;

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
