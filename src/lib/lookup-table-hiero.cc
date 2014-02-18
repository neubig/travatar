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
    	TranslationRuleHiero * rule = new TranslationRuleHiero(); 
    	BuildRule(rule, source_word, target_word, features);

    	Sentence src_sent = rule->GetSourceSentence();
    	Sentence trg_sent = rule->GetTrgWords();

    	int p=0;
    	while (p < (int)src_sent.size() && src_sent[p] < 0) p++; // finding position of terminal symbol;
    	ret->AddRule(src_sent[p], rule);
    }
    return ret;
}

TranslationRuleHiero * LookupTableHiero::BuildRule(TranslationRuleHiero * rule, vector<string> & source, 
		vector<string> & target, SparseMap features) 
{
	ostringstream source_string;
	for (int i=0; i < (int) source.size(); ++i) {
		int id = Dict::QuotedWID(source[i]);
		rule->AddSourceWord(id);

		if (i) source_string << " ";
		source_string << source[i];
	}

	for (int i=0; i < (int) target.size(); ++i) {
		int id = Dict::QuotedWID(target[i]);
		rule->AddTrgWord(id);
	}
	rule->SetSrcStr(source_string.str());
	rule->SetFeatures(features);
	return rule;
}

void LookupTableHiero::AddRule(WordId rule_starting_word, TranslationRuleHiero * rule) {
	if (rule_map.find(rule_starting_word) == rule_map.end()) {
		rule_map.insert(make_pair<int,vector<TranslationRuleHiero*> >(rule_starting_word, std::vector<TranslationRuleHiero*>()));
	}
	if (rule_map.find(rule_starting_word) != rule_map.end()) {
		(rule_map.find(rule_starting_word)->second).push_back(rule);
	} else {
		THROW_ERROR("Error when adding rule.");
	}
}

vector<TranslationRuleHiero*> & LookupTableHiero::FindRules(WordId input) {
	if (rule_map.find(input) == rule_map.end()){
		THROW_ERROR("Cannot find rule");
	} else {
		return (rule_map.find(input))->second;
	}
}

string LookupTableHiero::ToString() {
	std::ostringstream oss;
	RuleMapHiero::iterator it = rule_map.begin();
	int i=0;
	while(it != rule_map.end() && i++ < (int) rule_map.size()) {
		vector<TranslationRuleHiero*> vt = it->second;
		BOOST_FOREACH(TranslationRuleHiero* pt, vt) {
			oss << pt->ToString() << endl;
		}
		++it;
	}
	return oss.str();
}

HyperGraph * LookupTableHiero::BuildHyperGraph(string input) {
	HyperGraph * ret = new HyperGraph;
	map<int, HyperNode*> node_cache = map<int, HyperNode*>();

	Sentence sent = Dict::ParseWords(input);


	for (int index = 0; index < (int)sent.size(); ++index) {
		vector<TranslationRuleHiero*> rule = FindRules(sent[index]);
		BOOST_FOREACH(TranslationRuleHiero* r, rule) {
			int n_term = r->GetNumberOfNonTerminals();
			Sentence source = r -> GetSourceSentence();
			if (n_term == 0) {
				HyperNode* target = FindNode(node_cache, index, index+1);
				HyperEdge* rule_edge = new HyperEdge(target);
				rule_edge-> SetRule(r);
				target->AddEdge(rule_edge);
			} else {
				int head = 0;
				int tail = ((int) source.size())-1;

				vector<int> head_temp = vector<int>();
				stack<int> tail_temp = stack<int>();
				while (source[head] != sent[index]) { head_temp.push_back(head++); }
				while (source[tail] < 0) { tail_temp.push(tail--); } 
				
			}
		}
	}
	return ret;
}

HyperNode* LookupTableHiero::FindNode(map<int, HyperNode*> & _map, int begin, int end) {
	int value = Hash(begin,end);
	if (_map.find(value) == _map.end()) {
		return new HyperNode;
	} else {
		return (_map.find(value))->second;
	}
}