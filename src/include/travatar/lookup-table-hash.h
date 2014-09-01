#ifndef LOOKUP_TABLE_HASH_H__
#define LOOKUP_TABLE_HASH_H__

#include <travatar/lookup-table.h>
#include <boost/unordered_set.hpp>
#include <vector>

namespace travatar {

class HyperNode;

// A table that allows rules to be looked up in a hash table
class LookupTableHash : public LookupTable {
public:
    LookupTableHash() { }
    virtual ~LookupTableHash();

    virtual LookupState * GetInitialState() const {
        return new LookupState;
    }

    static LookupTableHash * ReadFromFile(std::string & filename);
    static LookupTableHash * ReadFromRuleTable(std::istream & in);

    // Find rules associated with a particular source pattern
    virtual const std::vector<TranslationRule*> * FindRules(const LookupState & state) const {
        RuleMap::const_iterator it = 
            rules_.find(state.GetString());
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

    LookupState * MatchState(const std::string & next, const LookupState & state) const;

    void AddRule(const std::string & str, TranslationRule * rule);

protected:
    boost::unordered_set<std::string> src_matches;
    typedef boost::unordered_map<std::string, std::vector<TranslationRule*> > RuleMap;
    typedef std::pair<const std::string, std::vector<TranslationRule*> > RulePair;
    RuleMap rules_;

};

}

#endif
