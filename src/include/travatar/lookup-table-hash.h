#ifndef LOOKUP_TABLE_HASH_H__
#define LOOKUP_TABLE_HASH_H__

#include <vector>
#include <tr1/unordered_set>
#include <boost/foreach.hpp>
#include <travatar/lookup-table.h>

namespace travatar {

class HyperNode;

// A single state for a partial rule match
class LookupStateHash : public LookupState {
public:
    LookupStateHash() : LookupState() { }
    virtual ~LookupStateHash() { }

    const std::string & GetString() const { return curr_string_; }
    void SetString(const std::string & str) { curr_string_ = str; }
protected:
    // A string representing the current progress
    std::string curr_string_;
};

// A table that allows rules to be looked up in a hash table
class LookupTableHash : public LookupTable {
public:
    LookupTableHash() { }
    virtual ~LookupTableHash() {
        BOOST_FOREACH(RulePair & rule_pair, rules_)
            BOOST_FOREACH(TranslationRule * rule, rule_pair.second)
                delete rule;
    };

    virtual LookupState * GetInitialState() const {
        return new LookupStateHash;
    }

    static LookupTableHash * ReadFromRuleTable(std::istream & in);

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const {
        RuleMap::const_iterator it = 
            rules_.find(((const LookupStateHash &)state).GetString());
        if(it == rules_.end())
            return NULL;
        else
            return &(it->second);
    }

    void AddToMatches(const std::string & str) {
        src_matches.insert(str);
    }

protected:

    // Match a single node
    virtual LookupState * MatchNode(const HyperNode & node, const LookupState & state) const;

    // Match the start of an edge
    virtual LookupState * MatchStart(const HyperNode & node, const LookupState & state) const;
    
    // Match the end of an edge
    virtual LookupState * MatchEnd(const HyperNode & node, const LookupState & state) const;

    LookupStateHash * MatchState(const std::string & next, const LookupState & state) const;

    void AddRule(TranslationRule * rule);

protected:
    std::tr1::unordered_set<std::string> src_matches;
    typedef std::tr1::unordered_map<std::string, std::vector<TranslationRule*> > RuleMap;
    typedef std::pair<const std::string, std::vector<TranslationRule*> > RulePair;
    RuleMap rules_;

};

}

#endif
