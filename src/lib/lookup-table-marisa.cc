#include <travatar/lookup-table-marisa.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

LookupTableMarisa * LookupTableMarisa::ReadFromRuleTable(std::istream & in) {
    // First read into a map
    typedef tr1::unordered_map< std::string, vector<TranslationRule*> > RuleMap;
    RuleMap rule_map;
    string line;
    LookupTableMarisa * ret = new LookupTableMarisa;
    while(getline(in, line)) {
        vector<string> columns;
        algorithm::split_regex(columns, line, regex(" \\|\\|\\| "));
        vector<WordId> trg_words = Dict::ParseQuotedWords(columns[1]);
        SparseMap features = Dict::ParseFeatures(columns[2]);
        rule_map[columns[0]].push_back(new TranslationRule(columns[0], trg_words, features));
    }
    // Next, convert this map into a vector and index
    marisa::Keyset keyset;
    BOOST_FOREACH(RuleMap::value_type & rule_val, rule_map)
        keyset.push_back(rule_val.first.c_str());
    // Build the trie
    ret->GetTrie().build(keyset);
    // Insert the rule arrays into the appropriate position based on the tree ID
    vector<vector<TranslationRule*> > & rules = ret->GetRules();
    rules.resize(keyset.size());
    marisa::Agent agent;
    agent.set_query("");
    while(ret->GetTrie().predictive_search(agent)) {
        string key(agent.key().ptr(), agent.key().length());
        rules[agent.key().id()] = rule_map[key];
    }
    return ret;
}

// Match a single node
LookupState * LookupTableMarisa::MatchNode(const HyperNode & node, const LookupState & state) {
    LookupStateMarisa * ret = NULL;
    const LookupStateMarisa & marisa_state = (const LookupStateMarisa &) state;
    if(node.IsTerminal()) {
        string next = marisa_state.GetString() + " \"" + Dict::WSym(node.GetSym()) + "\""; 
        ret = MatchState(next, state);
    } else {
        ostringstream next;
        next << marisa_state.GetString() << " x" << state.GetNonterms().size() << ":" << Dict::WSym(node.GetSym());
        ret = MatchState(next.str(), state);
        if(ret != NULL)
            ret->GetNonterms().push_back(&node);
    }
    return ret;
}

LookupStateMarisa * LookupTableMarisa::MatchState(const string & next, const LookupState & state) {
    marisa::Agent agent;
    agent.set_query(next.c_str());
    if(trie_.predictive_search(agent)) {
        // cerr << "Matching " << next << " --> success!" << endl;
        LookupStateMarisa * ret = new LookupStateMarisa;
        ret->SetString(next);
        ret->SetNonterms(state.GetNonterms());
        return ret;
    } else {
        // cerr << "Matching " << next << " --> failure!" << endl;
        return NULL;
    }
}


const vector<TranslationRule*> * LookupTableMarisa::FindRules(const LookupState & state) const {
    marisa::Agent agent;
    const char* query = ((const LookupStateMarisa &)state).GetString().c_str();
    agent.set_query(query);
    const vector<TranslationRule*> * ret = trie_.lookup(agent) ? &rules_[agent.key().id()] : NULL;
    return ret;
}
