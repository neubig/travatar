#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <travatar/lookup-table-fsm.h>
#include <travatar/sentence.h>
#include <sstream>

using namespace travatar;
using namespace std;
using namespace boost;

///////////////////////////////////
///     LOOK UP TABLE FSM        //
///////////////////////////////////
LookupTableFSM * LookupTableFSM::ReadFromRuleTable(std::istream & in) {
	string line;
    LookupTableFSM * ret = new LookupTableFSM;
    
    while(getline(in, line)) {
        vector<string> columns, source_word, target_word;
        algorithm::split_regex(columns, line, regex(" \\|\\|\\| "));
        if(columns.size() < 3) { delete ret; THROW_ERROR("Bad line in rule table: " << line << " [" << columns.size() << "]"); }

        algorithm::split(source_word, columns[0], is_any_of(" "));
        algorithm::split(target_word, columns[1], is_any_of(" "));
        SparseMap features = Dict::ParseFeatures(columns[2]);
    	TranslationRuleHiero * rule = new TranslationRuleHiero(); 
        BuildRule(rule, source_word, target_word, features);
        ret->AddRule(rule);
    	rule = NULL;
    }
    return ret;
}

TranslationRuleHiero * LookupTableFSM::BuildRule(TranslationRuleHiero * rule, vector<string> & source, 
		vector<string> & target, SparseMap features) 
{
	ostringstream source_string;
	int source_nt_count = 0;
    int target_nt_count = 0;
    int i;
	for (i=0; i < (int) source.size(); ++i) {
		if (source[i] == "@") break;
		int id = Dict::QuotedWID(source[i]);
		if (id < 0) {
			++source_nt_count;
			int k = source[i].size()-1;
			while (source[i][k-1] != ':' && k > 1) --k;
			rule->AddSourceWord(id,Dict::WID(source[i].substr(k,source[i].size()-k)));
		} else {
			rule->AddSourceWord(id);
		}
		if (i) source_string << " ";
		source_string << source[i];
	}

	for (i=0; i < (int) target.size(); ++i) {
		if (target[i] == "@") break;
		int id = Dict::QuotedWID(target[i]);
		if (id < 0) ++target_nt_count; 
		rule->AddTrgWord(id);
	}
	
	rule->SetSrcStr(source_string.str());
	rule->SetFeatures(features);
	rule->SetLabel(Dict::WID(target[i+1]));

	if (source_nt_count != target_nt_count) {
        cerr << rule->ToString() << endl;
        cerr << "Source: " << source_nt_count << endl;
        cerr << "Target: " << target_nt_count << endl;
		THROW_ERROR("Invalid rule. NT in source side != NT in target side");
	} else if (source_nt_count == 1 && (int)source.size() == 1 + 2) { // 1 + @ [ROOT]
		int k = source[0].size() - 1;
		while (source[0][k-1] != ':' && k > 1) --k;
		string label = source[0].substr(k,source[0].size()-k);
		// Case of Unary rule, that makes loop in the hypergraph
		if (label == target[i+1]) {
			cerr << rule->ToString() << endl;
			THROW_ERROR("Invalid rule. Travatar does not allow unary rule with same label (it will create loop in hypergraph).");
		}
	}
	return rule;
}

void LookupTableFSM::AddRule(TranslationRuleHiero* rule) {
	if (rule->GetSourceSentence().size() < 0) {
		THROW_ERROR("Error when reading rule on lookup-table-fsm. Rule string cannot be empty.");
	} 
	LookupTableFSM::AddRule(0,root_node, rule);
}

void LookupTableFSM::AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule) {
    Sentence rule_sent = rule->GetSourceSentence();
    WordId key = rule_sent[position];

    if (key < 0) {
        key = -1;
    }

    LookupNodeFSM* next_node = target_node->FindNode(key);
    if (next_node == NULL) {
    	next_node = new LookupNodeFSM;
    	target_node->AddEntry(key, next_node);
    }
    if (position+1 == (int)rule_sent.size()) {
    	next_node->AddRule(rule);
    } else {
    	AddRule(position+1, next_node, rule);
    }
}

LookupTableFSM::HieroRuleSpans* LookupTableFSM::GetSpanCopy(const LookupTableFSM::HieroRuleSpans spans) const {
	pair<int,int> pair_temp;
	LookupTableFSM::HieroRuleSpans* rule_span = new LookupTableFSM::HieroRuleSpans();
	BOOST_FOREACH(pair_temp, spans) {
		rule_span->push_back(pair_temp);
	}
	return rule_span;
}

HyperGraph * LookupTableFSM::TransformGraph(const HyperGraph & graph) const {
	HyperGraph* _graph = new HyperGraph;
	Sentence sent = graph.GetWords();
	LookupTableFSM::HieroRuleSpans span = LookupTableFSM::HieroRuleSpans();
	LookupTableFSM::NodeMap node_map = LookupTableFSM::NodeMap();
	LookupTableFSM::EdgeList edge_list = LookupTableFSM::EdgeList(); 
	BuildHyperGraphComponent(node_map, edge_list, sent, root_node, 0, -1, span);
	bool checkup_unknown[sent.size()];
	vector<LookupTableFSM::TailSpanKey > temp_spans;
	if (!GetDeleteUnknown()) {
		for (LookupTableFSM::NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it) {
			HyperNode* node = it->second;
			pair<int,int> node_span = node->GetSpan();
			if ((node_span.second - node_span.first) == 1) {
				int i = node_span.first;
				checkup_unknown[i] = true;
				if ((int)node->GetEdges().size() == 0) {
					TranslationRuleHiero* unk_rule = GetUnknownRule(sent[i],(it->first).first);
					HyperEdge* unk_edge = TransformRuleIntoEdge(&node_map,i,i+1,temp_spans,unk_rule);
					_graph->AddEdge(unk_edge);
					unk_edge = NULL;
					delete unk_rule;
				}
			}
		}
	}
	// Adding Unknown Edge and Adding word
	for (int i=0; i < (int) sent.size(); ++i) {
		// word i in the sentence is unknown!
		if (!GetDeleteUnknown() && !checkup_unknown[i]) {
			pair<int,int> word_span = make_pair(i,i+1);
			TranslationRuleHiero* unk_rule = GetUnknownRule(sent[i],GetDefaultSymbol());
			HyperEdge* unk_edge = TransformRuleIntoEdge(&node_map,i,i+1,temp_spans,unk_rule);
			_graph->AddEdge(unk_edge);
			unk_edge = NULL;
			delete unk_rule;
		}
		_graph->AddWord(sent[i]);
	}

	// Cleaning unreachable Node
	CleanUnreachableNode(edge_list, node_map);
	// First place the root node in the first (and if there is only one node, then don't have to do this)
	if (node_map.size() != 1) {
		LookupTableFSM::NodeKey key = make_pair(GetRootSymbol(),make_pair(0,(int)sent.size()));
		LookupTableFSM::NodeMap::iterator big_span_node = node_map.find(key);
		if (big_span_node != node_map.end()) {
			_graph->AddNode(big_span_node->second);
			node_map.erase(big_span_node);
		} else {
			big_span_node = node_map.begin();
			while (big_span_node != node_map.end()) {
				pair<int,int> key = (big_span_node->first).second;
				if (key.second - key.first == (int)sent.size()) {
					_graph->AddNode(big_span_node->second);
					node_map.erase(big_span_node);
					break;
				}
				++big_span_node;
			}
		}
	}
	BOOST_FOREACH (HyperEdge* edges, edge_list) 
		if (edges != NULL)
			_graph->AddEdge(edges);

	for (LookupTableFSM::NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it) 
		_graph->AddNode(it->second);

	return _graph;
}

void LookupTableFSM::BuildHyperGraphComponent(LookupTableFSM::NodeMap & node_map, 
		LookupTableFSM::EdgeList & edge_list, const Sentence & input,  LookupNodeFSM* node, int position, 
		int last_scan, LookupTableFSM::HieroRuleSpans & spans) const 
{
	if (position < (int)input.size()) {
		pair<int,int> temp_pair;
		LookupNodeFSM* x_node = node->FindNode(-1);
		if (x_node != NULL) {
			LookupTableFSM::HieroRuleSpans rule_span_next = LookupTableFSM::HieroRuleSpans();
			BOOST_FOREACH(temp_pair, spans) { rule_span_next.push_back(temp_pair); }
			rule_span_next.push_back(make_pair(last_scan+1,position+1));

			BuildHyperGraphComponent(node_map, edge_list, input, x_node, position+1, position, rule_span_next);
			BuildHyperGraphComponent(node_map, edge_list, input, node, position+1, last_scan, spans);

			BOOST_FOREACH(TranslationRuleHiero* rule, x_node->GetTranslationRules()) {
				if (rule->GetSourceSentence()[0] < 0) {
					int limit = ((int)rule_span_next.size() > 1) ? rule_span_next[1].first : (int)input.size();
					for (int i=0; i < limit; ++i) {
						if (limit - i <= span_length) {
							LookupTableFSM::HieroRuleSpans nt_front_rs = LookupTableFSM::HieroRuleSpans();
							nt_front_rs.push_back(make_pair(i,limit));
							for (int j=1; j < (int)rule_span_next.size(); ++j) 
								nt_front_rs.push_back(rule_span_next[j]);
							// { /* DEBUG */  cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, nt_front_rs) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; }
							edge_list.push_back(TransformRuleIntoEdge(rule,nt_front_rs,node_map));
						}	
					}
				} else if (NTInSpanLimit(rule, rule_span_next)) {
					// { /* DEBUG */  cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, rule_span_next) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; } 
					edge_list.push_back(TransformRuleIntoEdge(rule, rule_span_next,node_map));
				}
			}
		}

		LookupNodeFSM* next_node = node->FindNode(input[position]);
		if (next_node != NULL && ((int)spans.size() == 0 || spans[spans.size()-1].second == position)) {
			LookupTableFSM::HieroRuleSpans rule_span_next = LookupTableFSM::HieroRuleSpans();
			BOOST_FOREACH(temp_pair, spans) { rule_span_next.push_back(temp_pair); }
			rule_span_next.push_back(make_pair(position,position+1));

			BuildHyperGraphComponent(node_map, edge_list, input, next_node, position+1, position, rule_span_next);
			
			BOOST_FOREACH(TranslationRuleHiero* rule, next_node->GetTranslationRules()) {
				if (rule->GetSourceSentence()[0] < 0) {
					int limit = ((int)rule_span_next.size() > 1) ? rule_span_next[1].first : (int)input.size();
					for (int i=0; i < limit; ++i) {
						if (limit - i <= span_length) {
							LookupTableFSM::HieroRuleSpans nt_front_rs = LookupTableFSM::HieroRuleSpans();
							nt_front_rs.push_back(make_pair(i,limit));
							for (int j=1; j < (int)rule_span_next.size(); ++j) 
								nt_front_rs.push_back(rule_span_next[j]);
							// { /* DEBUG */ cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, nt_front_rs) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; }
							edge_list.push_back(TransformRuleIntoEdge(rule,nt_front_rs,node_map));
						}	
					}
				} else if (NTInSpanLimit(rule, rule_span_next)) {
					// { /* DEBUG */  cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, rule_span_next) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; } 
					edge_list.push_back(TransformRuleIntoEdge(rule, rule_span_next,node_map));
				}
			}
		}
	}
}

void LookupTableFSM::CleanUnreachableNode(LookupTableFSM::EdgeList & input, LookupTableFSM::NodeMap & node_map) const {
	bool removed;
	do {
		removed = false;
		for (LookupTableFSM::NodeMap::iterator it = node_map.begin(); it != node_map.end();) {
			vector<HyperEdge*>* node_edge = &it->second->GetEdges();
			for (int i = node_edge->size()-1; i >= 0; --i) {
				HyperEdge* now = (*node_edge)[i];
				vector<HyperNode*> node_child = now->GetTails();
				bool edge_valid = true;
				for (int j=0; edge_valid && j < (int)node_child.size(); ++j) {
					if (node_child[j]->GetEdges().size() == 0) {
						edge_valid = false;
					}
				}
				if (!edge_valid) {
					removed = true;
					node_edge->erase(node_edge->begin()+i);
				}
				now = NULL;
			}
			if (node_edge->size() == 0) {
				node_map.erase(it++);
			} else {
				++it;
			}
			node_edge = NULL;
		}	
	} while (removed);
	for (int i=input.size()-1; i >= 0 ; --i) {
		if (input[i]->GetHead()->GetEdges().size() == 0) {
			delete input[i];
			input[i] = NULL;
		}
	}
}


bool LookupTableFSM::NTInSpanLimit(TranslationRuleHiero* rule, const HieroRuleSpans & spans) const {
	bool ret = true;
	if (rule->GetSourceSentence()[0] < 0) {
		ret = (spans[0].second - spans[0].first) <= span_length;
	}
	if (ret && rule->GetSourceSentence()[(int)spans.size()-1]) {
		pair<int,int> back_span = spans[(int)spans.size()-1];
		ret = (back_span.second - back_span.first) <= span_length;
	}
	return ret;
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(TranslationRuleHiero* rule, const LookupTableFSM::HieroRuleSpans & rule_span,
	LookupTableFSM::NodeMap & node_map) const
{
	std::vector<int> non_term_position = rule->GetNonTermPositions();
	std::vector<LookupTableFSM::TailSpanKey > span_temp;
	for (int i=0 ; i < (int)non_term_position.size(); ++i) {
		span_temp.push_back(make_pair(i,rule_span[non_term_position[i]]));
	}
	int head_first = rule_span[0].first;
	int head_second = rule_span[(int)rule_span.size()-1].second;
	HyperEdge* edge = TransformRuleIntoEdge(&node_map, head_first, head_second, span_temp, rule);

	return edge;
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(LookupTableFSM::NodeMap* node_map, 
		const int head_first, const int head_second, const vector<LookupTableFSM::TailSpanKey > & tail_spans, 
		TranslationRuleHiero* rule) const
{
	HyperEdge* hedge = new HyperEdge;
	HyperNode* head = FindNode(node_map, head_first, head_second, rule->GetLabel());
	hedge->SetHead(head);
	hedge->SetRule(rule, rule->GetFeatures());
	hedge->SetRuleStr(rule->ToString());
	head->AddEdge(hedge);
	LookupTableFSM::TailSpanKey tail_span;
	BOOST_FOREACH(tail_span, tail_spans) {
		HyperNode* tail = FindNode(node_map, tail_span.second.first, tail_span.second.second, rule->GetChildNTLabel(tail_span.first));
		tail->SetSpan(tail_span.second);
		hedge->AddTail(tail);
		tail = NULL;
	}
	head = NULL;
	return hedge;
}

// Build a HyperEdge for a rule, also constructing node if head or tails node are not in the map.
// Then attaching rule into the edge

// Get an HyperNode, indexed by its span in some map.
HyperNode* LookupTableFSM::FindNode(LookupTableFSM::NodeMap* map_ptr, 
		const int span_begin, const int span_end, const WordId label) const
{
	if (span_begin < 0 || span_end < 0) 
		THROW_ERROR("Invalid span range in constructing HyperGraph.");
	pair<int,int> span = make_pair(span_begin,span_end);

	LookupTableFSM::NodeKey key = make_pair(label, span);
	LookupTableFSM::NodeMap::iterator it = map_ptr->find(key);
	if (it != map_ptr->end()) {
		return it->second;
	} else {
		// Fresh New Node!
		HyperNode* ret = new HyperNode;
		ret->SetSpan(make_pair(span_begin,span_end));
		ret->SetSym(label);
		map_ptr->insert(make_pair(key,ret));
		return ret;
	}
}

std::string LookupTableFSM::ToString() const {
	return root_node->ToString();
}

TranslationRuleHiero* LookupTableFSM::GetUnknownRule(WordId unknown_word, WordId label) const 
{
	TranslationRuleHiero* unknown_rule = new TranslationRuleHiero();
    SparseMap features = Dict::ParseFeatures("unk=1");
    unknown_rule->SetFeatures(features);
    unknown_rule->AddSourceWord(unknown_word);
    unknown_rule->AddTrgWord(unknown_word);
    unknown_rule->SetSrcStr(Dict::WSym(unknown_word));
    unknown_rule->SetLabel(label);
    return unknown_rule;
}

///////////////////////////////////
///     LOOK UP NODE FSM         //
///////////////////////////////////
void LookupNodeFSM::AddEntry(WordId & key, LookupNodeFSM* rule) {
	lookup_map[key] = rule;
}

void LookupNodeFSM::AddRule(TranslationRuleHiero* rule) {
	rules.push_back(rule);
}

LookupNodeFSM* LookupNodeFSM::FindNode(WordId key) const {
	NodeMap::const_iterator it = lookup_map.find(key); 
	if (it != lookup_map.end()) {
		return it->second;
	} else {
		return NULL;
	}
}

std::string LookupNodeFSM::ToString() const {
	return ToString(0);
}

std::string LookupNodeFSM::ToString(int indent) const {
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
