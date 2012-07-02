#include <travatar/lookup-table-hash.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

LookupTableHash * LookupTableHash::ReadFromRuleTable(std::istream & in) {
    string line;
    LookupTableHash * ret = new LookupTableHash;
    while(getline(in, line)) {
        vector<string> columns, words;
        algorithm::split_regex(columns, line, regex(" \\|\\|\\| "));
        algorithm::split(words, columns[0], is_any_of(" "));
        ostringstream partial;
        int pos = 0;
        BOOST_FOREACH(const string & str, words) {
            if(pos++) partial << ' ';
            partial << str;
            ret->AddToMatches(partial.str());
        }
        vector<WordId> trg_words = Dict::ParseQuotedWords(columns[1]);
        vector<double> features;
        istringstream feat_in(columns[2]);
        double feat;
        while(feat_in >> feat)
            features.push_back(feat);
        ret->AddRule(new TranslationRule(columns[0], trg_words, features));
    }
    return ret;
}

// Match a single node
LookupState * LookupTableHash::MatchNode(const HyperNode & node, const LookupState & state) {
    LookupStateHash * ret = NULL;
    const LookupStateHash & hash_state = (const LookupStateHash &) state;
    if(node.IsTerminal()) {
        string next = hash_state.GetString() + " \"" + Dict::WSym(node.GetSym()) + "\""; 
        ret = MatchState(next, state);
    } else {
        ostringstream next;
        next << hash_state.GetString() << " x" << state.GetNonterms().size() << ":" << Dict::WSym(node.GetSym());
        ret = MatchState(next.str(), state);
        if(ret != NULL)
            ret->GetNonterms().push_back(&node);
    }
    return ret;
}
