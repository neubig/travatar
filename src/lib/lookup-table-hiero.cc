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


///////////////////////////////////
///     LOOK UP TABLE HIERO      //
///////////////////////////////////
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
    	rule = BuildRule(rule, source_word, target_word, features);
    	ret->AddRule(rule);
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


HyperGraph * LookupTableHiero::BuildHyperGraph(string & input) {
	// TODO IMPLEMENTING HYPERGRAPH
	THROW_ERROR("Build HyperGraph is not implemented yet.");
	return new HyperGraph;
}

void LookupTableHiero::AddRule(TranslationRuleHiero* rule) {
	LookupTableHiero::AddRule(0,root_node, rule);
}


void LookupTableHiero::AddRule(int position, LookupNodeHiero* target_node, TranslationRuleHiero* rule) {
	Sentence source = rule->GetSourceSentence();
	std::vector<WordId> key_id = std::vector<WordId>();

	// Skip all non-terminal symbol
	while (source[position] < 0 && position < (int) source.size()) ++position;
	
	// Scanning all terminal symbols as key
	while (source[position] > 0 && position < (int) source.size()) {
		key_id.push_back(source[position++]);
	}

	GenericString<WordId> key = GenericString<WordId>(key_id);
	LookupNodeHiero* child_node = target_node->FindNode(key); 
	if (child_node == NULL) {
		child_node = new LookupNodeHiero;
		target_node->AddEntry(key, child_node);
	} 
	
	if (position+1 < (int) source.size()) {
		AddRule(position, child_node, rule);
	} else {
		child_node->AddRule(rule);
	}
}


std::string LookupTableHiero::ToString() {
	return root_node->ToString();
}

std::vector<TranslationRuleHiero*> LookupTableHiero::FindRules(const Sentence & input) const {
	return FindRules(root_node,input,0);
}

std::vector<TranslationRuleHiero*> LookupTableHiero::FindRules(LookupNodeHiero* node, const Sentence & input, int start) const {
	std::vector<TranslationRuleHiero*> result = std::vector<TranslationRuleHiero*>();

	// For All Possible Phrase
	for (int i=start; i < (int)input.size(); ++i) {
		for (int j=i; j < (int)input.size(); ++j) {
			std::vector<WordId> temp_key = std::vector<WordId>();

			// Get The substring
			for (int k=i; k<=j; ++k) temp_key.push_back(input[k]);
			GenericString<WordId> key_substr = GenericString<WordId>(temp_key);

			// Find node corresponds to the key
			LookupNodeHiero* result_node = node->FindNode(key_substr);

			// We found the node [hash]
			if (result_node != NULL) {
				std::vector<TranslationRuleHiero*> temp = result_node->GetTranslationRules();
				// For every Translation rule in that node
				BOOST_FOREACH(TranslationRuleHiero* r, temp) {
					Sentence src_sent = r->GetSourceSentence();
					// Consider if the rule contains non terminal in the beginning or in the end.
					// If either is true, then the corresponding phrase must not reach end of the sentence
					// or not in the beginning of the sentence according to which condition it meets.
					bool start_nt_front = i==0 && src_sent[i] < 0;
					bool reach_nt_end = j==(int)input.size()-1 && src_sent[src_sent.size()-1] < 0;

					if (!start_nt_front && !reach_nt_end)
					{
						r->ClearNonTermSpan();
						if (src_sent[0] < 0) {
							r-> AddNonTermSpanInFront(-1, i);
						} 
						if (src_sent[src_sent.size()-1] < 0) {
							r-> AddNonTermSpanInEnd(j+1,-1);
						}
						result.push_back(r);
					}
				}

				// Recursively finding node and skipping the non-terminal, starting with fresh terminal symbol .
				int skip = 1;
				GenericString<WordId> now_key = GenericString<WordId>(input[j+skip]);
				// Scan as many terminals as possible
				while (result_node->FindNode(now_key) == NULL && j+skip < (int) input.size()) {
					temp_key.clear();
					temp_key.push_back(input[j+ ++skip]);
					now_key = GenericString<WordId>(temp_key);
				}

				// Find All rules in the child scanning from forward position
				temp = FindRules(result_node, input, j+skip);
				// That rule in the child has a nonterminal symbol that is scanned in parent.
				// We have to include them also.
				BOOST_FOREACH(TranslationRuleHiero* r, temp) {
					r->AddNonTermSpanInFront(j+1,j+skip);
					result.push_back(r);
				}
			}
		}
	}
	return result;
}

///////////////////////////////////
///     LOOK UP NODE HIERO       //
///////////////////////////////////
void LookupNodeHiero::AddEntry(GenericString<WordId> & key, LookupNodeHiero* rule) {
	lookup_map[key] = rule;
}

void LookupNodeHiero::AddRule(TranslationRuleHiero* rule) {
	rules.push_back(rule);
}


LookupNodeHiero* LookupNodeHiero::FindNode(GenericString<WordId> & key) {
	NodeMap::iterator it = lookup_map.find(key); 
	if (it != lookup_map.end()) {
		return it->second;
	} else {
		return NULL;
	}
}

std::string LookupNodeHiero::ToString() {
	return ToString(0);
}

std::string LookupNodeHiero::ToString(int indent) {
	ostringstream str;
	for (int i=0; i < indent; ++i) str << " ";
	str << "===================================" << endl;
	BOOST_FOREACH(TranslationRuleHiero* rule, rules) {
		for (int i=0; i < indent; ++i) str << " ";
		str << rule->ToString() << endl;
	}
	for (int i=0; i < indent; ++i) str << " ";
	str << "===================================" << endl;
	NodeMap::iterator it = lookup_map.begin();
	while (it != lookup_map.end()) {
		string t_str = it->second->ToString(indent+1);
		str << t_str << endl;
		++it;
	}	
	return str.str();
}