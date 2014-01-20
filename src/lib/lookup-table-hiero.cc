#include <travatar/translation-rule-hiero.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <travatar/lookup-table-hiero.h>
#include <travatar/sentence.h>
#include <sstream>

using namespace travatar;
using namespace std;
using namespace boost;

LookupTableHiero * LookupTableHiero::ReadFromRuleTable(std::istream & in) {
	string line;
    LookupTableHiero * ret = new LookupTableHiero;
    while(getline(in, line)) {
        vector<string> columns, source_word, target_word;
        algorithm::split_regex(columns, line, regex(" \\|\\|\\| "));
        if(columns.size() < 3) { delete ret; THROW_ERROR("Bad line in rule table: " << line); }

        algorithm::split(source_word, columns[0], is_any_of(" "));
        algorithm::split(target_word, columns[1], is_any_of(" "));
        SparseMap features = Dict::ParseFeatures(columns[2]);
    	TranslationRuleHiero rule; 
    	BuildRule(rule, source_word, target_word, features);

    	Sentence src_sent = rule.GetSourceSentence();
    	Sentence trg_sent = rule.GetTargetSentence();

    	int p=0;
    	while (p < (int)src_sent.size() && src_sent[p] < 0) p++; // finding position of terminal symbol;
    	ret->AddRule(src_sent[p], rule);
    }
    return ret;
}

TranslationRuleHiero & LookupTableHiero::BuildRule(TranslationRuleHiero & rule, vector<string> & source, 
		vector<string> & target, SparseMap features) 
{
	for (int i=0; i < (int) source.size(); ++i) {
		int id = Dict::QuotedWID(source[i]);
		if (id < 0) {
			// case of nonterminal
			if (i == 0) {
				//beginning of the word
				rule.AddSourceWord(id,make_pair<int,int>(-1,1));
			} else if (i == (int) source.size() -1) {
				// end of the word
				rule.AddSourceWord(id,make_pair<int,int>(i,-1));
			} else {
				// non terminal in the middle
				rule.AddSourceWord(id,make_pair<int,int>(i,i+1));
			}
		} else {
			rule.AddSourceWord(id,make_pair<int,int>(i,i+1));
		}
	}

	for (int i=0; i < (int) target.size(); ++i) {
		int id = Dict::QuotedWID(target[i]);
		if (id < 0) {
			// case of nonterminal
			if (i == 0) {
				//beginning of the word
				rule.AddSourceWord(id,make_pair<int,int>(-1,1));
			} else if (i == (int) target.size() -1) {
				// end of the word
				rule.AddSourceWord(id,make_pair<int,int>(i,-1));
			} else {
				// non terminal in the middle
				rule.AddSourceWord(id,make_pair<int,int>(i,i+1));
			}
		} else {
			rule.AddSourceWord(id,make_pair<int,int>(i,i+1));
		}
	}
	rule.SetFeatures(features);
	return rule;
}

void LookupTableHiero::AddRule(WordId rule_starting_word, TranslationRuleHiero rule) {
	if (rule_map.find(rule_starting_word) == rule_map.end()) {
		rule_map.insert(make_pair<int,vector<TranslationRuleHiero*> >(rule_starting_word, std::vector<TranslationRuleHiero*>()));
	}
	(rule_map.find(rule_starting_word)->second).push_back(&rule);
}

vector<TranslationRuleHiero*> & LookupTableHiero::FindRules(WordId input) {
	return (rule_map.find(input))->second;
}