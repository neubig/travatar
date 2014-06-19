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
LookupTableFSM * LookupTableFSM::ReadFromRuleTable(istream & in) {
	string line;
    LookupTableFSM * ret = new LookupTableFSM;
    while(getline(in, line)) {
        vector<string> columns;
        algorithm::split_regex(columns, line, regex(" \\|\\|\\| "));
        if(columns.size() < 3)
            THROW_ERROR("Wrong number of columns in rule table, expected at least 3 but got "<<columns.size()<<": " << endl << line);
        CfgData src_data = Dict::ParseAnnotatedWords(columns[0]);
    	TranslationRuleHiero * rule = new TranslationRuleHiero(
            columns[0],
            Dict::ParseAnnotatedVector(columns[1]),
            Dict::ParseFeatures(columns[2]),
            src_data
        ); 
        // Sanity check
        BOOST_FOREACH(const CfgData & trg_data, rule->GetTrgData())
            if(trg_data.syms.size() != src_data.syms.size())
                THROW_ERROR("Mismatched number of non-terminals in rule table: " << endl << line);
        if(src_data.words.size() == 0)
            THROW_ERROR("Empty sources in a rule are not allowed: " << endl << line);
        if(src_data.syms.size() == 1 && src_data.words.size() == 1 && src_data.syms[0] == src_data.label)
            THROW_ERROR("Unary rules with identical labels for parent and child are not allowed: " << endl << line);
        // Add the rule
    	ret->AddRule(rule);
    }
    return ret;
}

void LookupTableFSM::AddRule(TranslationRuleHiero* rule) {
	LookupTableFSM::AddRule(0, root_node_, rule);
}

void LookupTableFSM::AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule) {
    Sentence rule_sent = rule->GetSrcData().words;
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
	BuildHyperGraphComponent(node_map, edge_list, sent, root_node_, 0, -1, span);
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
    
    // Find the root node
	LookupTableFSM::NodeKey key = make_pair(GetRootSymbol(),make_pair(0,(int)sent.size()));
	LookupTableFSM::NodeMap::iterator big_span_node = node_map.find(key);

    // If the node is not found, delete and return an empty graph
    if(big_span_node == node_map.end()) {
        cerr << "Could not find Span "<<Dict::WSym(GetRootSymbol())<<"[0,"<<sent.size()<<"]"<<endl;
	    BOOST_FOREACH (HyperEdge* edges, edge_list) 
            delete edges;
        BOOST_FOREACH (LookupTableFSM::NodeMap::value_type nodes, node_map)
            delete nodes.second;
        return new HyperGraph;
    }

    // Add the root node
	if (big_span_node != node_map.end()) {
		_graph->AddNode(big_span_node->second);
		node_map.erase(big_span_node);
	}
    // Add the rest of the nodes
	BOOST_FOREACH (HyperEdge* edges, edge_list) 
		_graph->AddEdge(edges);
    BOOST_FOREACH (LookupTableFSM::NodeMap::value_type nodes, node_map)
		_graph->AddNode(nodes.second);

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
				if (rule->GetSrcData().words[0] < 0) {
					int limit = ((int)rule_span_next.size() > 1) ? rule_span_next[1].first : (int)input.size();
					for (int i=0; i < limit; ++i) {
						if (limit - i <= span_length_) {
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
				if (rule->GetSrcData().words[0] < 0) {
					int limit = ((int)rule_span_next.size() > 1) ? rule_span_next[1].first : (int)input.size();
					for (int i=0; i < limit; ++i) {
						if (limit - i <= span_length_) {
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
	if (rule->GetSrcData().words[0] < 0) {
		ret = (spans[0].second - spans[0].first) <= span_length_;
	}
	if (ret && rule->GetSrcData().words[(int)spans.size()-1]) {
		pair<int,int> back_span = spans[(int)spans.size()-1];
		ret = (back_span.second - back_span.first) <= span_length_;
	}
	return ret;
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(TranslationRuleHiero* rule, const LookupTableFSM::HieroRuleSpans & rule_span,
	LookupTableFSM::NodeMap & node_map) const
{
	vector<int> non_term_position = rule->GetSrcData().GetNontermPositions();
	vector<LookupTableFSM::TailSpanKey > span_temp;
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
	HyperNode* head = FindNode(node_map, head_first, head_second, rule->GetSrcData().label);
	hedge->SetHead(head);
	hedge->SetRule(rule, rule->GetFeatures());
	hedge->SetRuleStr(rule->ToString());
	head->AddEdge(hedge);
	LookupTableFSM::TailSpanKey tail_span;
	BOOST_FOREACH(tail_span, tail_spans) {
		HyperNode* tail = FindNode(node_map, tail_span.second.first, tail_span.second.second, rule->GetSrcData().GetSym(tail_span.first));
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

string LookupTableFSM::ToString() const {
	return root_node_->ToString();
}

TranslationRuleHiero* LookupTableFSM::GetUnknownRule(WordId unknown_word, WordId label) const 
{
	return new TranslationRuleHiero(
        "UNK",
        CfgDataVector(GlobalVars::trg_factors, CfgData(Sentence(1, unknown_word), label)),
        Dict::ParseFeatures("unk=1"),
        CfgData(Sentence(1, unknown_word), label)
    );
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

string LookupNodeFSM::ToString() const {
	return ToString(0);
}

string LookupNodeFSM::ToString(int indent) const {
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
