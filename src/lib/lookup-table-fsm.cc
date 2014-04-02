#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <boost/algorithm/string.hpp>
#include <travatar/tree-io.h>
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
        rule = BuildRule(rule, source_word, target_word, features);
        
    	if (rule->CheckNTSourceTargetEqual()) {
    		ret->AddRule(rule);
    	} else {
    		cerr << "Ignoring rule " << rule->ToString() << " because #NT in source != #NT in target." << endl;
    	}
    	rule = NULL;
    }
    return ret;
}

TranslationRuleHiero * LookupTableFSM::BuildRule(TranslationRuleHiero * rule, vector<string> & source, 
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

void LookupTableFSM::AddRule(TranslationRuleHiero* rule) {
	if (rule->GetSourceSentence().size() < 0) {
		THROW_ERROR("Error when reading rule on lookup-table-fsm. Rule string cannot be empty.");
	} 
	LookupTableFSM::AddRule(0,root_node, rule);
}

void LookupTableFSM::AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule) {
    Sentence rule_sent = rule->GetSourceSentence();
    WordId key = rule_sent[position];

    // Generalize all non terminal symbols into X
    if (key < 0) key = -1;

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
	LookupTableFSM::EdgeSet edge_set = LookupTableFSM::EdgeSet(); 
	_graph = BuildHyperGraph(_graph, node_map, edge_set, sent, root_node, 0, -1, span, false);

	// Adding Unknown Edge and Adding word
	vector<pair<int,int> > temp_spans;
	for (int i=0; i < (int) sent.size(); ++i) {
		pair<int,int> word_span = make_pair(i,i+1);
		LookupTableFSM::NodeMap::iterator it = node_map.find(word_span);
		if (it == node_map.end()) {
			HyperNode* word_node = new HyperNode;
			word_node->SetSpan(word_span);
			it = node_map.insert(make_pair(word_span, word_node)).first;
			word_node = NULL;
		}
		HyperNode* node = it->second;
		if (node->GetEdges().size() == (unsigned) 0) {
			TranslationRuleHiero* unk_rule = GetUnknownRule(sent[i]);
			HyperEdge* unk_edge = TransformRuleIntoEdge(&node_map,i,i+1,temp_spans,unk_rule);
			_graph->AddEdge(unk_edge);
			unk_edge = NULL;
			delete unk_rule;
		}
		_graph->AddWord(sent[i]);
		node = NULL;
	}

	// ADDING GLUE RULE 
	vector<pair<int,int> > span_temp;
	for (int i=2; i <= (int)sent.size(); ++i) {
		AddGlueRule(0,i,_graph,&node_map,&span_temp,&edge_set);
	}

	// First place the root node in the first (and if there is only one node, then don't have to do this)
	if (node_map.size() != 1) {
		LookupTableFSM::NodeMap::iterator big_span_node = node_map.find(make_pair(0,(int)sent.size()));
		_graph->AddNode(big_span_node->second);
		node_map.erase(big_span_node);
	}

	for (LookupTableFSM::NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it) 
		_graph->AddNode(it->second);
		
	return _graph;
}

HyperGraph * LookupTableFSM::BuildHyperGraph(HyperGraph* ret, LookupTableFSM::NodeMap & node_map, 
		LookupTableFSM::EdgeSet & edge_set, const Sentence & input,  LookupNodeFSM* node, int position, 
		int last_scan, LookupTableFSM::HieroRuleSpans & spans, bool only_x) const 
{
	if (position < (int)input.size()) {
		pair<int,int> temp_pair;
		LookupNodeFSM* x_node = node->FindNode(-1);
		if (x_node != NULL) {
			LookupTableFSM::HieroRuleSpans rule_span_next = LookupTableFSM::HieroRuleSpans();
			BOOST_FOREACH(temp_pair, spans) { rule_span_next.push_back(temp_pair); }
			rule_span_next.push_back(make_pair(last_scan+1,position+1));

			BuildHyperGraph(ret, node_map, edge_set, input, x_node, position+1, position, rule_span_next, false);
			BuildHyperGraph(ret, node_map, edge_set, input, node, position+1, last_scan, spans, false);

			BOOST_FOREACH(TranslationRuleHiero* rule, x_node->GetTranslationRules()) {
				if (rule->GetSourceSentence()[0] < 0) {
					int limit = ((int)rule_span_next.size() > 1) ? rule_span_next[1].first : (int)input.size();
					for (int i=0; i < limit; ++i) {
						if (limit - i < span_length) {
							LookupTableFSM::HieroRuleSpans nt_front_rs = LookupTableFSM::HieroRuleSpans();
							nt_front_rs.push_back(make_pair(i,limit));
							for (int j=1; j < (int)rule_span_next.size(); ++j) 
								nt_front_rs.push_back(rule_span_next[j]);
							// { /* DEBUG */  cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, nt_front_rs) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; }
							ret->AddEdge(TransformRuleIntoEdge(rule,nt_front_rs,node_map,edge_set));
						}	
					}
				} else if (NTInSpanLimit(rule, rule_span_next)) {
					// { /* DEBUG */  cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, rule_span_next) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; } 
					ret->AddEdge(TransformRuleIntoEdge(rule, rule_span_next,node_map,edge_set));
				}
			}
		}

		if (!only_x) {
			LookupNodeFSM* next_node = node->FindNode(input[position]);
			if (next_node != NULL && ((int)spans.size() == 0 || spans[spans.size()-1].second == position)) {
				LookupTableFSM::HieroRuleSpans rule_span_next = LookupTableFSM::HieroRuleSpans();
				BOOST_FOREACH(temp_pair, spans) { rule_span_next.push_back(temp_pair); }
				rule_span_next.push_back(make_pair(position,position+1));

				BuildHyperGraph(ret, node_map, edge_set, input, next_node, position+1, position, rule_span_next, false);
				
				BOOST_FOREACH(TranslationRuleHiero* rule, next_node->GetTranslationRules()) {
					if (rule->GetSourceSentence()[0] < 0) {
						int limit = ((int)rule_span_next.size() > 1) ? rule_span_next[1].first : (int)input.size();
						for (int i=0; i < limit; ++i) {
							if (limit - i < span_length) {
								LookupTableFSM::HieroRuleSpans nt_front_rs = LookupTableFSM::HieroRuleSpans();
								nt_front_rs.push_back(make_pair(i,limit));
								for (int j=1; j < (int)rule_span_next.size(); ++j) 
									nt_front_rs.push_back(rule_span_next[j]);
								// { /* DEBUG */ cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, nt_front_rs) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; }
								ret->AddEdge(TransformRuleIntoEdge(rule,nt_front_rs,node_map,edge_set));
							}	
						}
					} else if (NTInSpanLimit(rule, rule_span_next)) {
						// { /* DEBUG */  cerr << rule->ToString() << "\t"; BOOST_FOREACH(temp_pair, rule_span_next) cerr << "(" << temp_pair.first << "," << temp_pair.second << ")"; cerr << endl; } 
						ret->AddEdge(TransformRuleIntoEdge(rule, rule_span_next,node_map,edge_set));
					}
				}
			}
		}
	}
	return ret;
}

bool LookupTableFSM::NTInSpanLimit(TranslationRuleHiero* rule, const HieroRuleSpans & spans) const {
	bool ret = true;
	if (rule->GetSourceSentence()[0] < 0) {
		ret = (spans[0].second - spans[0].first) < span_length;
	}
	if (ret && rule->GetSourceSentence()[(int)spans.size()-1]) {
		pair<int,int> back_span = spans[(int)spans.size()-1];
		ret = (back_span.second - back_span.first) < span_length;
	}
	return ret;
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(TranslationRuleHiero* rule, const LookupTableFSM::HieroRuleSpans & rule_span,
	LookupTableFSM::NodeMap & node_map, LookupTableFSM::EdgeSet & edge_set) const
{
	std::vector<int> non_term_position = rule->GetNonTermPositions();
	std::vector<pair<int,int> > span_temp;
	BOOST_FOREACH(int nt, non_term_position) {
		span_temp.push_back(rule_span[nt]);
	}
	int head_first = rule_span[0].first;
	int head_second = rule_span[(int)rule_span.size()-1].second;
	HyperEdge* edge = TransformRuleIntoEdge(&node_map, head_first, head_second, span_temp, rule);
	edge_set.insert(TransformSpanToKey(head_first,head_second,span_temp));	
	return edge;
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(LookupTableFSM::NodeMap* node_map, 
		const int head_first, const int head_second, const vector<pair<int,int> > & tail_spans, 
		TranslationRuleHiero* rule) const
{
	HyperEdge* hedge = new HyperEdge;
	HyperNode* head = FindNode(node_map, head_first, head_second);
	hedge->SetHead(head);
	hedge->SetRule(rule, rule->GetFeatures());
	hedge->SetRuleStr(rule->ToString());
	head->AddEdge(hedge);
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


LookupTableFSM::EdgeKey LookupTableFSM::TransformSpanToKey(const int head_start, const int head_end, 
		const vector<pair<int,int> > & tail_spans) const 
{
	vector<int> temp;
	temp.push_back(head_start);
	temp.push_back(head_end);
	pair<int,int> tail;
	BOOST_FOREACH(tail, tail_spans) {
		temp.push_back(tail.first);
		temp.push_back(tail.second);
	}
	return EdgeKey(temp);
}

void LookupTableFSM::AddGlueRule(int start, int end, HyperGraph* graph, 
	LookupTableFSM::NodeMap* node_map, vector<pair<int,int> >* span_temp, LookupTableFSM::EdgeSet * edge_set) const 
{
	if (start+1 < end) {		
		for (int i=start+1; i < end; ++i) {
			// creating tail
			if (end - i <= span_length) { 
				span_temp->clear();
				span_temp->push_back(pair<int,int>(start,i));
				span_temp->push_back(pair<int,int>(i,end));
				LookupTableFSM::EdgeKey span_key = TransformSpanToKey(start,end,*span_temp);
				if (edge_set->find(span_key) == edge_set->end()) {
					edge_set->insert(span_key);
					graph->AddEdge(TransformRuleIntoEdge(node_map, start,end, *span_temp, glue_rule));
				}
			}
		}
	}
}

// Build a HyperEdge for a rule, also constructing node if head or tails node are not in the map.
// Then attaching rule into the edge

// Get an HyperNode, indexed by its span in some map.
HyperNode* LookupTableFSM::FindNode(map<pair<int,int>, HyperNode*>* map_ptr, 
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

std::string LookupTableFSM::ToString() const {
	return root_node->ToString();
}

TranslationRuleHiero* LookupTableFSM::GetUnknownRule(WordId unknown_word) const 
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
