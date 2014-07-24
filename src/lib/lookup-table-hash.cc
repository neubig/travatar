#include <travatar/translation-rule.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/input-file-stream.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>
#include <boost/foreach.hpp>
#include <sstream>

using namespace travatar;
using namespace std;
using namespace boost;

LookupTableHash::~LookupTableHash() {
    BOOST_FOREACH(RulePair & rule_pair, rules_)
        BOOST_FOREACH(TranslationRule * rule, rule_pair.second)
            delete rule;
};

// Match the start of an edge
LookupState * LookupTableHash::MatchStart(const HyperNode & node, const LookupState & state) const {
    const std::string & p = ((const LookupStateHash &)state).GetString();
    std::string next = p + (p.size()?" ":"") + Dict::WSym(node.GetSym()) + " (";
    return MatchState(next, state);
}

// Match the end of an edge
LookupState * LookupTableHash::MatchEnd(const HyperNode & node, const LookupState & state) const {
    std::string next = ((const LookupStateHash &)state).GetString() + " )";
    return MatchState(next, state);
}

LookupStateHash * LookupTableHash::MatchState(const std::string & next, const LookupState & state) const {
    if(src_matches.find(next) != src_matches.end()) {
        // std::cerr << "Matching " << next << " --> success!" << std::endl;
        LookupStateHash * ret = new LookupStateHash;
        ret->SetString(next);
        ret->SetNonterms(state.GetNonterms());
        ret->SetFeatures(state.GetFeatures());
        return ret;
    } else {
        // std::cerr << "Matching " << next << " --> failure!" << std::endl;
        return NULL;
    }
}

void LookupTableHash::AddRule(const std::string & str, TranslationRule * rule) {
    rules_[str].push_back(rule);
}


LookupTableHash * LookupTableHash::ReadFromFile(std::string & filename) {
    InputFileStream tm_in(filename.c_str());
    cerr << "Reading TM file from "<<filename<<"..." << endl;
    if(!tm_in)
        THROW_ERROR("Could not find TM: " << filename);
    return ReadFromRuleTable(tm_in);
}

LookupTableHash * LookupTableHash::ReadFromRuleTable(std::istream & in) {
    string line;
    LookupTableHash * ret = new LookupTableHash;
    while(getline(in, line)) {
        vector<string> columns = Tokenize(line, " ||| ");
        if(columns.size() < 3) { delete ret; THROW_ERROR("Bad line in rule table: " << line); }
        vector<string> words = Tokenize(columns[0], ' ');
        ostringstream partial;
        int pos = 0;
        BOOST_FOREACH(const string & str, words) {
            if(pos++) partial << ' ';
            partial << str;
            ret->AddToMatches(partial.str());
        }
        CfgDataVector trg_data = Dict::ParseAnnotatedVector(columns[1]);
        SparseVector features = Dict::ParseSparseVector(columns[2]);
        ret->AddRule(columns[0], new TranslationRule(trg_data, features));
    }
    return ret;
}

// Match a single node
LookupState * LookupTableHash::MatchNode(const HyperNode & node, const LookupState & state) const {
    LookupStateHash * ret = NULL;
    const LookupStateHash & hash_state = (const LookupStateHash &) state;
    if(node.IsTerminal()) {
        string next = hash_state.GetString() + " \"" + Dict::WSym(node.GetSym()) + "\""; 
        ret = MatchState(next, state);
    } else {
        ostringstream next;
        next << hash_state.GetString() << " x" << state.GetNonterms().size() << ":" << Dict::WSym(node.GetSym());
        ret = MatchState(next.str(), state);
        if(ret != NULL) {
            ret->GetNonterms().push_back(&node);
            ret->SetFeatures(state.GetFeatures());
        }
    }
    return ret;
}
