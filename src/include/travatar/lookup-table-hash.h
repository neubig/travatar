#ifndef LOOKUP_TABLE_HASH_H__
#define LOOKUP_TABLE_HASH_H__

#include <vector>
#include <travatar/hyper-graph.h>
#include <travatar/lookup-table.h>
#include <boost/foreach.hpp>
#include <boost/tr1/unordered_set.hpp>

namespace travatar {

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
    virtual ~LookupTableHash() { };

    virtual LookupState * GetInitialState() {
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
    virtual LookupState * MatchNode(const HyperNode & node, const LookupState & state);

    // Match the start of an edge
    virtual LookupState * MatchStart(const HyperNode & node, const LookupState & state) {
        const std::string & p = ((const LookupStateHash &)state).GetString();
        std::string next = p + (p.size()?" ":"") + Dict::WSym(node.GetSym()) + " (";
        return MatchState(next, state);
    }
    
    // Match the end of an edge
    virtual LookupState * MatchEnd(const HyperNode & node, const LookupState & state) {
        std::string next = ((const LookupStateHash &)state).GetString() + " )";
        return MatchState(next, state);
    }

    LookupStateHash * MatchState(const std::string & next, const LookupState & state) {
        if(src_matches.find(next) != src_matches.end()) {
            // std::cerr << "Matching " << next << " --> success!" << std::endl;
            LookupStateHash * ret = new LookupStateHash;
            ret->SetString(next);
            ret->SetNonterms(state.GetNonterms());
            return ret;
        } else {
            // std::cerr << "Matching " << next << " --> failure!" << std::endl;
            return NULL;
        }
    }

    void AddRule(TranslationRule * rule) {
        rules_[rule->GetSrcStr()].push_back(rule);
    }

protected:
    std::tr1::unordered_set<std::string> src_matches;
    typedef std::tr1::unordered_map<std::string, std::vector<TranslationRule*> > RuleMap;
    RuleMap rules_;

};

}

#endif
