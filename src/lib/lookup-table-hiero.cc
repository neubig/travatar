#include <travatar/translation-rule-hiero.h>
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
        if(columns.size() < 3)
            THROW_ERROR("Wrong number of columns in rule table, expected at least 3 but got "<<columns.size()<<": " << endl << line);
    	TranslationRuleHiero * rule = new TranslationRuleHiero(
            columns[0],
            Dict::ParseAnnotatedVector(columns[1]),
            Dict::ParseFeatures(columns[2]),
            Dict::ParseAnnotatedWords(columns[0])
        ); 
        // Sanity check
        BOOST_FOREACH(const CfgData & trg_data, rule->GetTrgData())
            if(trg_data.syms.size() != rule->GetSrcData().syms.size())
                THROW_ERROR("Mismatched number of non-terminals in rule table: " << endl << line);
    	ret->AddRule(rule);
    }
    return ret;
}

// TranslationRuleHiero * LookupTableHiero::BuildRule(TranslationRuleHiero * rule, vector<string> & source, 
// 		vector<string> & target, SparseMap features) 
// {
// 	ostringstream source_string;
// 	int source_nt_count = 0;
//     int target_nt_count = 0;
//     int i;
// 	for (i=0; i < (int) source.size(); ++i) {
// 		if (source[i] == "@") break;
// 		int id = Dict::QuotedWID(source[i]);
// 		if (id < 0) {
// 			++source_nt_count;
// 			int k = source[i].size()-1;
// 			while (source[i][k-1] != ':' && k > 1) --k;
// 			rule->AddSourceWord(id,Dict::WID(source[i].substr(k,source[i].size()-k)));
// 		} else {
// 			rule->AddSourceWord(id);
// 		}
// 		if (i) source_string << " ";
// 		source_string << source[i];
// 	}
// 
// 	for (i=0; i < (int) target.size(); ++i) {
// 		if (target[i] == "@") break;
// 		int id = Dict::QuotedWID(target[i]);
// 		if (id < 0) ++target_nt_count; 
// 		rule->AddTrgWord(id);
// 	}
// 	
// 	rule->SetSrcStr(source_string.str());
// 	rule->SetFeatures(features);
// 	rule->SetLabel(Dict::WID(target[i+1]));
// 
// 	if (source_nt_count != target_nt_count) {
//         cerr << rule->ToString() << endl;
//         cerr << "Source: " << source_nt_count << endl;
//         cerr << "Target: " << target_nt_count << endl;
// 		THROW_ERROR("Invalid rule. NT in source side != NT in target side");
// 	} else if (source_nt_count == 1 && (int)source.size() == 1 + 2) { // 1 + @ [ROOT]
// 		int k = source[0].size() - 1;
// 		while (source[0][k-1] != ':' && k > 1) --k;
// 		string label = source[0].substr(k,source[0].size()-k);
// 		// Case of Unary rule, that makes loop in the hypergraph
// 		if (label == target[i+1]) {
// 			cerr << rule->ToString() << endl;
// 			THROW_ERROR("Invalid rule. Travatar does not allow unary rule with same label (it will create loop in hypergraph).");
// 		}
// 	}
// 	return rule;
// }

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
		word_node = NULL;
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
					if (j-i <= span_length) {
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
						edge = NULL;
					}
				}
			}
		} else if (front_span.first == -1) {
			// iterating all possible values in head
			for (int i=0; i < front_span.second; ++i) {
				if (back_span.second - i <= span_length) {
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
					edge = NULL;
				}
			}
		} else if (back_span.second == -1) {
			// iterating all possible values in tail
			for (int j= back_span.first+1; j <= (int)sent.size(); ++j) {
				if (j-front_span.first <= span_length) {
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
					edge = NULL;
				}
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
			edge = NULL;
		}
		// No need to use that span anymore
		rule = NULL;
		delete item.second;
	}

	// ADDING GLUE RULE 
	for (int i=2; i <= (int)sent.size(); ++i) {
		AddGlueRule(0,i,ret,&node_map,&span_temp,&edge_set);
	}

	// Add all nodes constructed during adding edge into the hypergraph and add unknown edge to
	// node that doesn't have edge

	// First place the root node in the first (and if there is only one node, then don't have to do this)
	if (node_map.size() != 1) {
		map<pair<int,int>, HyperNode*>::iterator big_span_node = node_map.find(pair<int,int>(0,(int)sent.size()));
		ret->AddNode(big_span_node->second);
		node_map.erase(big_span_node);
	}

	map<pair<int,int>, HyperNode*>::iterator it = node_map.begin();
	while(it != node_map.end()) {
		HyperNode* node = (it++->second);
		// Add Unknown edge to node
		if (!GetDeleteUnknown() && (int)(node->GetEdges().size()) == 0) {
			pair<int,int> span = node->GetSpan();
			if (span.second - span.first == 1) {
				HyperEdge* unknown_edge = new HyperEdge;
				TranslationRuleHiero* unk_rule_ptr = GetUnknownRule(sent[span.first]);
				unknown_edge->SetHead(node);
				unknown_edge->SetRule(unk_rule_ptr);
				unknown_edge->SetRuleStr(unk_rule_ptr->ToString());
				delete unk_rule_ptr;
				node->AddEdge(unknown_edge);
				ret->AddEdge(unknown_edge);
				unknown_edge = NULL;
			}
		}
		ret->AddNode(node);
		node = NULL;
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
			if (end - i <= span_length) { 
				span_temp->clear();
				span_temp->push_back(pair<int,int>(start,i));
				span_temp->push_back(pair<int,int>(i,end));
				GenericString<WordId> span_key = TransformSpanToKey(start,end,*span_temp);
				if (edge_set->find(span_key) == edge_set->end()) {
					edge_set->insert(span_key);
					ret->AddEdge(TransformRuleIntoEdge(node_map, start,end, *span_temp, glue_rule));
				}
			}
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
	hedge->SetRuleStr(rule->ToString());
	head->AddEdge(hedge);
	
	// For each tail_spans, add them into the edge_tail.
	pair<int,int> tail_span;
	BOOST_FOREACH(tail_span, tail_spans) {
		HyperNode* tail = FindNode(node_map, tail_span.first, tail_span.second);
		tail->SetSpan(tail_span);
		hedge->AddTail(tail);
		tail = NULL;
	}
	head = NULL;
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
	int prev_position = position;
    Sentence source = rule->GetSourceSentence();
	std::vector<WordId> key_id = std::vector<WordId>();

	// Skip all non-terminal symbol
	while (position < (int) source.size() && source[position] < 0) ++position;
	
	// Scanning all terminal symbols as key
	while (position < (int) source.size() && source[position] > 0) {
		key_id.push_back(source[position++]);
	}

	GenericString<WordId> key = GenericString<WordId>(key_id);
	LookupNodeHiero* child_node = target_node->FindNode(key); 
	if (child_node == NULL) {
		child_node = new LookupNodeHiero;
		target_node->AddEntry(key, child_node);
	} 
	
    if (prev_position != position) {
	    if (position+1 < (int) source.size()) {
		    AddRule(position, child_node, rule);
	    } else {
		    child_node->AddRule(rule);
	    }
    }
	child_node = NULL;
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
						dq = NULL;
					}
				}

				// Recursively finding node and skipping the non-terminal, starting with fresh terminal symbol .
				int skip;
				GenericString<WordId> now_key = GenericString<WordId>(1);
                for(skip = 1; j+skip < (int)input.size(); skip++) {
                    now_key[0] = input[j+skip];
                    if(result_node->FindNode(now_key) == NULL)
                        break;
                }

				// Find All rules in the child scanning from forward position
				vector<pair<TranslationRuleHiero*, HieroRuleSpans* > >temp_result = FindRules(result_node, input, j+skip, depth+1);
				pair<TranslationRuleHiero*, HieroRuleSpans*> item;
				// That rule in the child has a nonterminal symbol that is scanned in parent.
				// We have to include them also.
				BOOST_FOREACH(item, temp_result) {
					item.second->push_front(pair<int,int>(j+1,(*(item.second))[0].first));
					for (int l=temp_key.size()-1; l>=0;--l) {
						item.second->push_front(pair<int,int>(l+i,l+i+1));
					}	
					if (depth == 0 && item.first->GetSourceSentence()[0] < 0) {
						item.second->push_front(pair<int,int>(-1,i));
					}
					result.push_back(item);
				}
				result_node = NULL;
			} else {
				break;
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
