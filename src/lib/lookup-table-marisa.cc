#include <travatar/lookup-table-marisa.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/input-file-stream.h>
#include <travatar/util.h>
#include <marisa/marisa.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>

using namespace travatar;
using namespace std;
using namespace boost;

// Match the start of an edge
LookupState * LookupTableMarisa::MatchStart(const HyperNode & node, const LookupState & state) const {
    const std::string & p = ((const LookupStateMarisa &)state).GetString();
    std::string next = p + (p.size()?" ":"") + Dict::WSym(node.GetSym()) + " (";
    return MatchState(next, state);
}

// Match the end of an edge
LookupState * LookupTableMarisa::MatchEnd(const HyperNode & node, const LookupState & state) const {
    std::string next = ((const LookupStateMarisa &)state).GetString() + " )";
    return MatchState(next, state);
}

LookupTableMarisa * LookupTableMarisa::ReadFromFile(std::string & filename) {
    InputFileStream tm_in(filename.c_str());
    cerr << "Reading TM file from "<<filename<<"..." << endl;
    if(!tm_in)
        THROW_ERROR("Could not find TM: " << filename);
    return ReadFromRuleTable(tm_in);
}

LookupTableMarisa * LookupTableMarisa::ReadFromRuleTable(std::istream & in) {
    // First read in the rule table
    string line;
    LookupTableMarisa * ret = new LookupTableMarisa;
    // Rule table
    typedef vector<TranslationRule*> RuleVec;
    vector<RuleVec> rules;
    marisa::Keyset keyset;
    while(getline(in, line)) {
        vector<string> columns = Tokenize(line, " ||| ");
        if(columns.size() < 3) { delete ret; THROW_ERROR("Bad line in rule table: " << line); }
        vector<WordId> trg_words, trg_syms;
        CfgDataVector trg_data = Dict::ParseAnnotatedVector(columns[1]);
        SparseVector features = Dict::ParseSparseVector(columns[2]);
        TranslationRule* rule = new TranslationRule(columns[0], trg_data, features);
        if(rules.size() == 0 || columns[0] != rules[rules.size()-1][0]->GetSrcStr()) {
            keyset.push_back(rule->GetSrcStr().c_str());
            rules.push_back(vector<TranslationRule*>());
        }
        rules[rules.size()-1].push_back(rule);
    }
    // Build the trie
    ret->GetTrie().build(keyset);
    // Insert the rule arrays into the appropriate position based on the tree ID
    vector<RuleVec> & main_rules = ret->GetRules();
    main_rules.resize(keyset.size());
    BOOST_FOREACH(const RuleVec & my_rules, rules) {
        marisa::Agent agent;
        agent.set_query(my_rules[0]->GetSrcStr().c_str());
        if(!ret->GetTrie().lookup(agent))
            THROW_ERROR("Internal error when building rule table @ " << my_rules[0]->GetSrcStr());
        main_rules[agent.key().id()] = my_rules;
    }
    return ret;
}

// Match a single node
LookupState * LookupTableMarisa::MatchNode(const HyperNode & node, const LookupState & state) const {
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

LookupStateMarisa * LookupTableMarisa::MatchState(const string & next, const LookupState & state) const {
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


LookupTableMarisa::~LookupTableMarisa() {
    BOOST_FOREACH(std::vector<TranslationRule*> & vec, rules_)
        BOOST_FOREACH(TranslationRule * rule, vec)
            delete rule;
};
