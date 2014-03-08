#include <travatar/translation-rule-hiero.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <boost/algorithm/string.hpp>
#include <travatar/tree-io.h>
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

HyperGraph * LookupTableHiero::TransformGraph(const HyperGraph & graph) const {
	HyperGraph* _graph = BuildHyperGraph(graph.GetWords());
	// DEBUG
	// vector<HyperNode*> _nodes = _graph->GetNodes();
	// vector<HyperEdge*> _edges = _graph->GetEdges();
	// cerr << "SIZE: " << _nodes.size() << " " << _edges.size() << endl;
	// BOOST_FOREACH(HyperNode* node, _nodes) {
	// 	cerr << *node << endl;
	// }
	// BOOST_FOREACH(HyperEdge* edge, _edges) {
	//	cerr << *edge << endl;
	// }
	return _graph;
}

HyperGraph * LookupTableHiero::BuildHyperGraph(const Sentence & sent) const {
	HyperGraph* ret = new HyperGraph;
	vector<pair<TranslationRuleHiero*, HieroRuleSpans* > > rules = FindRules(sent);
	vector<pair<int,int> > span_temp = std::vector<pair<int,int> >();
	map<pair<int,int>, HyperNode*> node_map = map<pair<int,int>, HyperNode*>();
	set<GenericString<WordId> > edge_set = set<GenericString<WordId> >(); 

	for (int i=0; i < (int) sent.size(); ++i) {
		HyperNode* word_node = new HyperNode;
		pair<int,int> node_span = pair<int,int>(i,i+1);
		word_node->SetSpan(node_span);
		node_map[node_span] = word_node;
		ret->AddWord(sent[i]);
	}

	pair<TranslationRuleHiero*, HieroRuleSpans* > item;
	// Transform every rule into edge and add it to hypergraph.
	BOOST_FOREACH(item, rules) {
		TranslationRuleHiero* rule = item.first;
		std::deque<pair<int,int> > rule_spans = *(item.second);
		pair<int,int> front_span = rule_spans[0];
		pair<int,int> back_span = rule_spans[(int)rule_spans.size()-1];
		vector<int> non_term_position = rule->GetNonTermPositions();

		// switch case whether there is a -1 symbol in the front or back of the rule
		if (front_span.first == -1 && back_span.second == -1) {
			// head and tail are non terminal symbol, iterating for all possible values.
			for (int i=0; i < front_span.second; ++i) {
				for (int j=back_span.first+1; j <= (int)sent.size(); ++j) {
					span_temp.clear();
					span_temp.push_back(std::pair<int,int>(i,front_span.second));
					BOOST_FOREACH(int position, non_term_position) {
						if (position != 0 && position != (int)rule_spans.size()-1) {
							span_temp.push_back(rule_spans[position]);
						}
					}
					span_temp.push_back(std::pair<int,int>(back_span.first,j));
					HyperEdge* edge = TransformRuleIntoEdge(&node_map, i, j, span_temp, rule);
					edge_set.insert(TransformSpanToKey(i,j,span_temp));
					ret->AddEdge(edge);
				}
			}
		} else if (front_span.first == -1) {
			// iterating all possible values in head
			for (int i=0; i < front_span.second; ++i) {
				span_temp.clear();
				span_temp.push_back(std::pair<int,int>(i,front_span.second));
				BOOST_FOREACH(int position, non_term_position) {
					if (position != 0) {
						span_temp.push_back(rule_spans[position]);
					}
				}
				HyperEdge* edge = TransformRuleIntoEdge(&node_map, i, back_span.second, span_temp, rule);
				edge_set.insert(TransformSpanToKey(i,back_span.second,span_temp));
				ret->AddEdge(edge);
			}
		} else if (back_span.second == -1) {
			// iterating all possible values in tail
			for (int j= back_span.first+1; j <= (int)sent.size(); ++j) {
				span_temp.clear();
				BOOST_FOREACH(int position, non_term_position) {
					if (position != (int) rule_spans.size()-1) {
						span_temp.push_back(rule_spans[position]);
					}
				}
				span_temp.push_back(std::pair<int,int>(back_span.first,j));
				HyperEdge* edge =TransformRuleIntoEdge(&node_map, front_span.first, j, span_temp, rule);
				edge_set.insert(TransformSpanToKey(front_span.first,j,span_temp));
				ret->AddEdge(edge);
			}
		} else {
			// nothing special, just single edge to be added
			span_temp.clear();
			BOOST_FOREACH(int position, non_term_position) {
				span_temp.push_back(rule_spans[position]);
			}
			HyperEdge* edge = TransformRuleIntoEdge(&node_map, front_span.first, back_span.second, span_temp, rule);
			edge_set.insert(TransformSpanToKey(front_span.first,back_span.second,span_temp));
			ret->AddEdge(edge);
		}
		// No need to use that span anymore
		delete item.second;
	}

	// ADDING GLUE RULE 
	for (int i=2; i <= (int)sent.size(); ++i) {
		AddGlueRule(0,i,ret,&node_map,&span_temp,&edge_set);
	}

	// Add all nodes constructed during adding edge into the hypergraph and add unknown edge to
	// node that doesn't have edge

	// First place the root node in the first (and if there is only one node doesn't have to do this)
	if (node_map.size() != 1) {
		map<pair<int,int>, HyperNode*>::iterator big_span_node = node_map.find(pair<int,int>(0,(int)sent.size()));
		ret->AddNode(big_span_node->second);
		node_map.erase(big_span_node);
	}

	map<pair<int,int>, HyperNode*>::iterator it = node_map.begin();
	while(it != node_map.end()) {
		HyperNode* node = it++->second;
		// Add Unknown edge to node
		if ((int)(node->GetEdges().size()) == 0) {
			pair<int,int> span = node->GetSpan();
			if (span.second - span.first == 1) {
				HyperEdge* unknown_edge = new HyperEdge;
				unknown_edge->SetHead(node);
				unknown_edge->SetRule(GetUnknownRule(sent[span.first]));
				node->AddEdge(unknown_edge);
				ret->AddEdge(unknown_edge);
			}
		}
		ret->AddNode(node);
	}
	
	return ret;
}

GenericString<WordId> LookupTableHiero::TransformSpanToKey(const int head_start, const int head_end, 
		const vector<pair<int,int> > & tail_spans) const 
{
	vector<WordId> temp;
	temp.push_back(head_start);
	temp.push_back(head_end);
	pair<int,int> tail;
	BOOST_FOREACH(tail, tail_spans) {
		temp.push_back(tail.first);
		temp.push_back(tail.second);
	}
	return GenericString<WordId>(temp);
}

void LookupTableHiero::AddGlueRule(int start, int end, HyperGraph* ret, 
	map<pair<int,int>, HyperNode*>* node_map, vector<pair<int,int> >* span_temp, set<GenericString<WordId> >* edge_set) const 
{
	if (start+1 < end) {		
		for (int i=start+1; i < end; ++i) {
			// creating tail
			span_temp->clear();
			span_temp->push_back(pair<int,int>(start,i));
			span_temp->push_back(pair<int,int>(i,end));
			GenericString<WordId> span_key = TransformSpanToKey(start,end,*span_temp);
			if (edge_set->find(span_key) == edge_set->end()) {
				edge_set->insert(span_key);
				ret->AddEdge(TransformRuleIntoEdge(node_map, start,end, *span_temp, glue_rule));
			}
			AddGlueRule(i, end, ret, node_map, span_temp, edge_set);
		}
	}
}

// Build a HyperEdge for a rule, also constructing node if head or tails node are not in the map.
// Then attaching rule into the edge
HyperEdge* LookupTableHiero::TransformRuleIntoEdge(map<pair<int,int>, HyperNode*>* node_map, 
		const int head_first, const int head_second, const vector<pair<int,int> > & tail_spans, 
		TranslationRuleHiero* rule) const
{
	HyperEdge* hedge = new HyperEdge;

	// First find the head.
	HyperNode* head = FindNode(node_map, head_first, head_second);
	
	// Attaching Edge to the head
	hedge->SetHead(head);
	hedge->SetRule(rule, rule->GetFeatures());
	head->AddEdge(hedge);
	
	// For each tail_spans, add them into the edge_tail.
	pair<int,int> tail_span;
	BOOST_FOREACH(tail_span, tail_spans) {
		HyperNode* tail = FindNode(node_map, tail_span.first, tail_span.second);
		tail->SetSpan(tail_span);
		hedge->AddTail(tail);
	}
	return hedge;
}

// Get an HyperNode, indexed by its span in some map.
HyperNode* LookupTableHiero::FindNode(map<pair<int,int>, HyperNode*>* map_ptr, 
		const int span_begin, const int span_end) const
{
	if (span_begin < 0 || span_end < 0) 
		THROW_ERROR("Invalid span range in constructing HyperGraph.");
	pair<int,int> span = std::pair<int,int>(span_begin,span_end);
	map<pair<int,int>, HyperNode*>::iterator it = map_ptr->find(span);
	if (it != map_ptr->end()) {
		return it->second;
	} else {
		// Fresh New Node!
		HyperNode* ret = new HyperNode;
		ret->SetSpan(std::pair<int,int>(span_begin,span_end));
		map_ptr->insert(std::pair<std::pair<int,int>, HyperNode*> (span,ret));
		return ret;
	}
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

std::string LookupTableHiero::ToString() const {
	return root_node->ToString();
}

std::vector<std::pair<TranslationRuleHiero*, HieroRuleSpans* > > LookupTableHiero::FindRules(const Sentence & input) const {
	return FindRules(root_node,input,0, 0);
}

std::vector<std::pair<TranslationRuleHiero*, HieroRuleSpans* > > LookupTableHiero::FindRules(LookupNodeHiero* node, 
		const Sentence & input, int start, int depth) const 
{
	std::vector<std::pair<TranslationRuleHiero*, HieroRuleSpans* > >  result = 
		std::vector<std::pair<TranslationRuleHiero*, HieroRuleSpans* > >();

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
						HieroRuleSpans* dq = new HieroRuleSpans;
						if (depth == 0 && src_sent[0] < 0) {
							dq->push_back(pair<int,int>(-1,i));
						}
						for (int l=0; l<(int)temp_key.size();++l) {
							dq->push_back(pair<int,int>(l+i,l+i+1));
						}
						if (src_sent[src_sent.size()-1] < 0) {
							dq->push_back(pair<int,int>(j+1,-1));
						}
						result.push_back(pair<TranslationRuleHiero*, HieroRuleSpans*> (r,dq));
					}
				}

				// Recursively finding node and skipping the non-terminal, starting with fresh terminal symbol .
				int skip = 1;
				GenericString<WordId> now_key = GenericString<WordId>(input[j+skip]);

				// Scan as many terminals as possible
				std::vector<WordId> now_key_sent = std::vector<WordId>();
				while (result_node->FindNode(now_key) == NULL && j+skip < (int) input.size()) {
					now_key_sent.clear();
					now_key_sent.push_back(input[j+ ++skip]);
					now_key = GenericString<WordId>(now_key_sent);
				}

				// Find All rules in the child scanning from forward position
				vector<pair<TranslationRuleHiero*, HieroRuleSpans* > >temp_result = FindRules(result_node, input, j+skip, depth+1);
				pair<TranslationRuleHiero*, HieroRuleSpans*> item;
				// That rule in the child has a nonterminal symbol that is scanned in parent.
				// We have to include them also.
				BOOST_FOREACH(item, temp_result) {
					item.second->push_front(pair<int,int>(j+1,j+skip));
					for (int l=temp_key.size()-1; l>=0;--l) {
						item.second->push_front(pair<int,int>(l+i,l+i+1));
					}	
					if (depth == 0 && item.first->GetSourceSentence()[0] < 0) {
						item.second->push_front(pair<int,int>(-1,i));
					}
					result.push_back(item);
				}
			}
		}
	}
	return result;
}

TranslationRuleHiero* LookupTableHiero::GetUnknownRule(WordId unknown_word) const 
{
	TranslationRuleHiero* unknown_rule = new TranslationRuleHiero();
    SparseMap features = Dict::ParseFeatures("unk=1");
    unknown_rule->SetFeatures(features);
    unknown_rule->AddSourceWord(unknown_word);
    unknown_rule->AddTrgWord(unknown_word);
    unknown_rule->SetSrcStr(Dict::WSym(unknown_word));
    return unknown_rule;
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


LookupNodeHiero* LookupNodeHiero::FindNode(GenericString<WordId> & key) const {
	NodeMap::const_iterator it = lookup_map.find(key); 
	if (it != lookup_map.end()) {
		return it->second;
	} else {
		return NULL;
	}
}

std::string LookupNodeHiero::ToString() const {
	return ToString(0);
}

std::string LookupNodeHiero::ToString(int indent) const {
	ostringstream str;
	for (int i=0; i < indent; ++i) str << " ";
	str << "===================================" << endl;
	BOOST_FOREACH(TranslationRuleHiero* rule, rules) {
		for (int i=0; i < indent; ++i) str << " ";
		str << rule->ToString() << endl;
	}
	for (int i=0; i < indent; ++i) str << " ";
	str << "===================================" << endl;
	NodeMap::const_iterator it = lookup_map.begin();
	while (it != lookup_map.end()) {
		string t_str = it->second->ToString(indent+1);
		str << t_str << endl;
		++it;
	}	
	return str.str();
}