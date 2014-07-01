#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <travatar/lookup-table-fsm.h>
#include <travatar/sentence.h>
#include <travatar/input-file-stream.h>
#include <sstream>

using namespace travatar;
using namespace std;
using namespace boost;

///////////////////////////////////
///     LOOK UP TABLE FSM        //
///////////////////////////////////
RuleFSM * RuleFSM::ReadFromRuleTable(istream & in) {
    string line;
    RuleFSM * ret = new RuleFSM;
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

void RuleFSM::AddRule(TranslationRuleHiero* rule) {
    RuleFSM::AddRule(0, root_node_, rule);
}

void RuleFSM::AddRule(int position, LookupNodeFSM* target_node, TranslationRuleHiero* rule) {
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

HieroRuleSpans* RuleFSM::GetSpanCopy(const HieroRuleSpans spans) const {
    pair<int,int> pair_temp;
    HieroRuleSpans* rule_span = new HieroRuleSpans();
    BOOST_FOREACH(pair_temp, spans) {
        rule_span->push_back(pair_temp);
    }
    return rule_span;
}

HyperGraph * LookupTableFSM::TransformGraph(const HyperGraph & graph) const {
    HyperGraph* _graph = new HyperGraph;
    Sentence sent = graph.GetWords();
    HieroRuleSpans span = HieroRuleSpans();
    HieroNodeMap node_map = HieroNodeMap();
    EdgeList edge_list = EdgeList(); 
    // For each starting point
    for(int i = sent.size()-1; i >= 0; i--) {
        // For each grammar
        BOOST_FOREACH(RuleFSM* rule_fsm, rule_fsms_) {
            rule_fsm->BuildHyperGraphComponent(node_map, edge_list, sent, rule_fsm->GetRootNode(), i, span);
        }
    }
    vector<bool> checkup_unknown(sent.size(), false);
    vector<TailSpanKey > temp_spans;
    BOOST_FOREACH(const HieroNodeMap::value_type & val, node_map) {
        HyperNode* node = val.second;
        pair<int,int> node_span = node->GetSpan();
        if ((node_span.second - node_span.first) == 1) {
            int i = node_span.first;
            if(node->GetEdges().size() == 0) {
                if (delete_unknown_) {
                    TranslationRuleHiero* unk_rule = GetUnknownRule(sent[i],val.first.first);
                    HyperEdge* unk_edge = LookupTableFSM::TransformRuleIntoEdge(&node_map,i,i+1,temp_spans,unk_rule);
                    _graph->AddEdge(unk_edge);
                    unk_edge = NULL;
                    delete unk_rule;
                }
            } else if(val.first.first == GetDefaultSymbol()) {
                // cerr << "checkup_unknown[" << i << "] == true" << endl;
                checkup_unknown[i] = true;
            }
        }
    }
    // Adding Unknown Edge and Adding word
    for (int i=0; i < (int) sent.size(); ++i) {
        // word i in the sentence is unknown!
        if (!delete_unknown_ && !checkup_unknown[i]) {
            TranslationRuleHiero* unk_rule = GetUnknownRule(sent[i],GetDefaultSymbol());
            HyperEdge* unk_edge = LookupTableFSM::TransformRuleIntoEdge(&node_map,i,i+1,temp_spans,unk_rule);
            _graph->AddEdge(unk_edge);
            unk_edge = NULL;
            delete unk_rule;
        }
        _graph->AddWord(sent[i]);
    }

    // Find the root node
    HieroNodeKey key = make_pair(GetRootSymbol(),make_pair(0,(int)sent.size()));
    HieroNodeMap::iterator big_span_node = node_map.find(key);

    // If the node is not found, delete and return an empty graph
    if(big_span_node == node_map.end()) {
        cerr << "Could not find Span "<<Dict::WSym(GetRootSymbol())<<"[0,"<<sent.size()<<"]"<<endl;
        BOOST_FOREACH (HyperEdge* edges, edge_list) 
            if(edges)
                delete edges;
        BOOST_FOREACH (HieroNodeMap::value_type nodes, node_map)
            delete nodes.second;
        return new HyperGraph;
    }

    // Cleaning unreachable Node
    CleanUnreachableNode(edge_list, node_map, big_span_node->second);

    // Add the root node
    if (big_span_node != node_map.end()) {
        _graph->AddNode(big_span_node->second);
        node_map.erase(big_span_node);
    }
    // Add the rest of the nodes
    BOOST_FOREACH (HyperEdge* edges, edge_list) 
        if(edges)
            _graph->AddEdge(edges);
    BOOST_FOREACH (HieroNodeMap::value_type nodes, node_map)
        _graph->AddNode(nodes.second);

    return _graph;
}

void RuleFSM::BuildHyperGraphComponent(
        HieroNodeMap & node_map, 
        EdgeList & edge_list, 
        const Sentence & input,
        LookupNodeFSM* node, 
        int position, 
        HieroRuleSpans & spans) const 
{
    if (position >= (int)input.size())
        return;

    // Processing of non-terminal nodes
    pair<int,int> temp_pair;
    LookupNodeFSM* x_node = node->FindNode(-1);
    if (x_node != NULL) {
        // Continue until the end of the sentence or the max span length
        int until = min((int)input.size(), position+span_length_);
        for(int next_pos = position+1; next_pos <= until; next_pos++) {
            // Add the rules
            HieroRuleSpans rule_span_next = HieroRuleSpans(spans);
            rule_span_next.push_back(make_pair(position,next_pos));
            BOOST_FOREACH(TranslationRuleHiero* rule, x_node->GetTranslationRules()) 
                edge_list.push_back(LookupTableFSM::TransformRuleIntoEdge(rule, rule_span_next, node_map));
            // Recurse to match the next node
            BuildHyperGraphComponent(node_map, edge_list, input, x_node, next_pos, rule_span_next);
        }
    }

    // For the nodes that match the words
    LookupNodeFSM* next_node = node->FindNode(input[position]);
    if (next_node != NULL) {
        // Add the new rules
        HieroRuleSpans rule_span_next = HieroRuleSpans(spans);
        rule_span_next.push_back(make_pair(position,position+1));
        BOOST_FOREACH(TranslationRuleHiero* rule, next_node->GetTranslationRules()) 
            edge_list.push_back(LookupTableFSM::TransformRuleIntoEdge(rule, rule_span_next, node_map));
        // Recurse to match the following rules
        BuildHyperGraphComponent(node_map, edge_list, input, next_node, position+1, rule_span_next);
    }
}

void LookupTableFSM::CleanUnreachableNode(EdgeList & input, HieroNodeMap & node_map, HyperNode* root) const {
    set<HyperNode*> deleted_nodes;
    set<HyperEdge*> deleted_edges;
    bool removed;
    do {
        removed = false;
        // Keep track of which nodes
        set<HyperNode*> has_head;
        has_head.insert(root);
        // For each node
        vector<HieroNodeMap::key_type> del_keys;
        for (HieroNodeMap::iterator it = node_map.begin(); it != node_map.end(); it++) {
            // Check if each edge in the node is OK
            vector<HyperEdge*> & node_edge = it->second->GetEdges();
            for (int i = node_edge.size()-1; i >= 0; --i) {
                HyperEdge* now = node_edge[i];
                // Find if at least one tail of the edge is bad
                vector<HyperNode*> node_child = now->GetTails();
                bool edge_valid = true;
                for (int j=0; edge_valid && j < (int)node_child.size(); ++j)
                    if (node_child[j]->GetEdges().size() == 0) 
                        edge_valid = false;
                // If the edge is OK, mark all of its children as having a head
                if (edge_valid) {
                    BOOST_FOREACH(HyperNode* child, node_child)
                        has_head.insert(child);
                // Otherwise, mark that at least one edge is removed and delete
                } else {
                    // cerr << "Removing edge from " << Dict::WSym(it->first.first) << it->first.second << endl;
                    removed = true;
                    deleted_edges.insert(now);
                    node_edge.erase(node_edge.begin()+i);
                }
            }
            if (node_edge.size() == 0) {
                // cerr << "Removing edgeless " << Dict::WSym(it->first.first) << it->first.second << endl;
                deleted_nodes.insert(it->second);
                del_keys.push_back(it->first);
            }   
        }    
        BOOST_FOREACH(HieroNodeMap::key_type & del, del_keys)
            node_map.erase(del);
        del_keys.resize(0);
        // If at least one node is missing a head, perform one more loop through and
        // remove all nodes without a head
        // cerr << "has_head.size() == " << has_head.size() << ", node_map.size() == " << node_map.size() << endl;
        if(has_head.size() != node_map.size()) {
            removed = true;
            for (HieroNodeMap::iterator it = node_map.begin(); it != node_map.end(); it++) {
                if(has_head.find(it->second) == has_head.end()) {
                    // cerr << "Removing "<<it->second->GetEdges().size()<<" edges from " << Dict::WSym(it->first.first) << it->first.second << endl;
                    // cerr << "Removing headless node " << Dict::WSym(it->first.first) << it->first.second << endl;
                    BOOST_FOREACH(HyperEdge* edge, it->second->GetEdges())
                        deleted_edges.insert(edge);
                    it->second->GetEdges().resize(0);
                    deleted_nodes.insert(it->second);
                    del_keys.push_back(it->first);
                }
            }
            BOOST_FOREACH(HieroNodeMap::key_type & del, del_keys)
                node_map.erase(del);
        }
    } while (removed);
    // Delete edges
    for (int i=input.size()-1; i >= 0 ; --i) {
        if (deleted_edges.find(input[i]) != deleted_edges.end()) {
            delete input[i];
            input[i] = NULL;
        }
    }
    // Delete nodes
    BOOST_FOREACH(HyperNode* del, deleted_nodes)
        delete del;
}


HyperEdge* LookupTableFSM::TransformRuleIntoEdge(TranslationRuleHiero* rule, const HieroRuleSpans & rule_span,
    HieroNodeMap & node_map)
{
    vector<int> non_term_position = rule->GetSrcData().GetNontermPositions();
    vector<TailSpanKey > span_temp;
    for (int i=0 ; i < (int)non_term_position.size(); ++i) {
        span_temp.push_back(make_pair(i,rule_span[non_term_position[i]]));
    }
    int head_first = rule_span[0].first;
    int head_second = rule_span[(int)rule_span.size()-1].second;
    HyperEdge* edge = TransformRuleIntoEdge(&node_map, head_first, head_second, span_temp, rule);

    return edge;
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(HieroNodeMap* node_map, 
        const int head_first, const int head_second, const vector<TailSpanKey> & tail_spans, 
        TranslationRuleHiero* rule)
{
    // // DEBUG start
    // cerr << " TransformRule @ " << make_pair(head_first,head_second) << " ->";
    // BOOST_FOREACH(const TailSpanKey & tsk, tail_spans) cerr << " " << Dict::WSym(rule->GetSrcData().GetSym(tsk.first)) << tsk.second;
    // cerr << " " << rule->GetSrcStr() << endl;
    // // DEBUG end

    HyperEdge* hedge = new HyperEdge;
    HyperNode* head = FindNode(node_map, head_first, head_second, rule->GetSrcData().label);
    hedge->SetHead(head);
    hedge->SetRule(rule, rule->GetFeatures());
    hedge->SetRuleStr(rule->ToString());
    head->AddEdge(hedge);
    TailSpanKey tail_span;
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
HyperNode* LookupTableFSM::FindNode(HieroNodeMap* map_ptr, 
        const int span_begin, const int span_end, const WordId label)
{
    if (span_begin < 0 || span_end < 0) 
        THROW_ERROR("Invalid span range in constructing HyperGraph.");
    pair<int,int> span = make_pair(span_begin,span_end);

    HieroNodeKey key = make_pair(label, span);
    HieroNodeMap::iterator it = map_ptr->find(key);
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

string RuleFSM::ToString() const {
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

LookupTableFSM * LookupTableFSM::ReadFromFiles(const std::vector<std::string> & filenames) {
    LookupTableFSM * ret = new LookupTableFSM;
    BOOST_FOREACH(const std::string & filename, filenames) {
        InputFileStream tm_in(filename.c_str());
        cerr << "Reading TM file from "<<filename<<"..." << endl;
        if(!tm_in)
            THROW_ERROR("Could not find TM: " << filename);
        ret->AddRuleFSM(RuleFSM::ReadFromRuleTable(tm_in));
    }
    return ret;
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
    HieroNodeMap::const_iterator it = lookup_map.find(key); 
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
    HieroNodeMap::const_iterator it = lookup_map.begin();
    while (it != lookup_map.end()) {
        string t_str = it->second->ToString(indent+1);
        str << t_str << endl;
        ++it;
    }    
    return str.str();
}
