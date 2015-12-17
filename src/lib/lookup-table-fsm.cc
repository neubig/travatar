#include <travatar/translation-rule-hiero.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>
#include <travatar/hyper-graph.h>
#include <travatar/lookup-table-fsm.h>
#include <travatar/sentence.h>
#include <travatar/input-file-stream.h>
#include <boost/foreach.hpp>
#include <sstream>

using namespace travatar;
using namespace std;
using namespace boost;

inline string PrintState(const string & str) {
    if(str.length() % sizeof(WordId) != 0)
        THROW_ERROR("Bad string length " << str.length());
    ostringstream oss;
    for(size_t i = 0; i < str.length(); i += sizeof(WordId)) {
        WordId wid = *(WordId*)&str[i];
        if(wid >= 0) {
            oss << '"' << Dict::WSym(wid) << "\" ";
        } else {
            oss << Dict::WSym(-1-wid) << " ";
        }
    }
    return oss.str();
}

///////////////////////////////////
///     LOOK UP TABLE FSM        //
///////////////////////////////////
LookupTableFSM::LookupTableFSM() : rule_fsms_(),
                   delete_unknown_(false),
                   trg_factors_(1),
                   root_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("S")))),
                   unk_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("X")))),
                   save_src_str_(false) { }

LookupTableFSM::~LookupTableFSM() {
    BOOST_FOREACH(RuleFSM* rule_fsm, rule_fsms_) {
        if(rule_fsm != NULL)
            delete rule_fsm;
    }
}

RuleFSM::~RuleFSM() {
    BOOST_FOREACH(RuleVec & vec, rules_)
        BOOST_FOREACH(TranslationRuleHiero * rule, vec)
            delete rule;
}

string RuleFSM::CreateKey(const CfgData & src_data,
                            const vector<CfgData> & trg_data) {
    // Convert the data so that for each word we get the word symbol
    // and for each non-terminal, we get -1-key for all of the targets
    // in order
    Sentence key_str;
    for(int i = 0; i < (int)src_data.words.size(); i++) {
        if(src_data.words[i] >= 0) {
            key_str.push_back(src_data.words[i]);
        } else {
            int pos = -1-src_data.words[i];
            key_str.push_back(-1-src_data.syms[pos]);
            BOOST_FOREACH(const CfgData & trg_datum, trg_data)
                key_str.push_back(-1-trg_datum.syms[pos]);
        }
    }
    return string((char*)&key_str[0], sizeof(WordId)*key_str.size());
}

RuleFSM * RuleFSM::ReadFromRuleTable(istream & in) {
    string line;
    RuleFSM * ret = new RuleFSM;
    UnaryMap & unaries = ret->unaries_;

    typedef unordered_map<string, vector<TranslationRuleHiero*> > RuleMap;
    RuleMap rules;

    while(getline(in, line)) {
        vector<string> columns = Tokenize(line, " ||| ");;
        if(columns.size() < 3)
            THROW_ERROR("Wrong number of columns in rule table, expected at least 3 but got "<<columns.size()<<": " << endl << line);
        CfgData src_data = Dict::ParseAnnotatedWords(columns[0]);
        vector<CfgData> trg_data = Dict::ParseAnnotatedVector(columns[1]);
        TranslationRuleHiero * rule = new TranslationRuleHiero(
            trg_data,
            Dict::ParseSparseVector(columns[2]),
            src_data
        );
        if(src_data.syms.size() == 1 && src_data.words.size() == 1)
            unaries[rule->GetChildHeadLabels(0)].insert(rule->GetHeadLabels());
        // Sanity check
        BOOST_FOREACH(const CfgData & trg_data, rule->GetTrgData())
            if(trg_data.syms.size() != src_data.syms.size())
                THROW_ERROR("Mismatched number of non-terminals in rule table: " << endl << line);
        if(src_data.words.size() == 0)
            THROW_ERROR("Empty sources in a rule are not allowed: " << endl << line);
        BOOST_FOREACH(WordId srcid, src_data.words)
            if(-1 - srcid > (int)src_data.syms.size())
                THROW_ERROR("Non-terminal ID is larger than number of symbols: " << endl << line);

        string key_str = CreateKey(src_data, trg_data);
        rules[key_str].push_back(rule);
    }

    // Build the key set
    marisa::Keyset keyset;
    BOOST_FOREACH(RuleMap::value_type & rule, rules)
        keyset.push_back(rule.first.c_str(), rule.first.length());

    // Expand unary values
    bool added = true;
    while(added) {
        added = false;
        BOOST_FOREACH(UnaryMap::value_type & val, unaries) {
            BOOST_FOREACH(HieroHeadLabels target, val.second) {
                if(val.first == target)
                    THROW_ERROR("Unary cycles are not allowed in CFG grammars, but found one with for label " << Dict::WSym(target[0]) << endl);
                UnaryMap::iterator it = unaries.find(target);
                if(it != unaries.end()) {
                    BOOST_FOREACH(HieroHeadLabels second_trg, it->second) {
                        set<HieroHeadLabels>::iterator it2 = val.second.find(target);
                        if(it2 == val.second.end()) {
                            added = true;
                            val.second.insert(second_trg);
                        } 
                    }
                }
            }
        }
    }
    // Compile the marisa trie
    ret->GetTrie().build(keyset);
    // Insert the rule arrays into the appropriate position based on the tree ID
    RuleSet & main_rules = ret->GetRules();
    main_rules.resize(keyset.size());
    BOOST_FOREACH(RuleMap::value_type & rule, rules) {
        marisa::Agent agent;
        agent.set_query(rule.first.c_str(), rule.first.length());
        if(!ret->GetTrie().lookup(agent))
            THROW_ERROR("Internal error when building rule table");
        main_rules[agent.key().id()].swap(rule.second);
        // BOOST_FOREACH(TranslationRuleHiero * hier, main_rules[agent.key().id()])
        //     cerr << "RULE: " << PrintState(rule.first) << " @ "<<agent.key().id()<<": " << *hier << endl;
    }
    return ret;
}

HyperGraph * LookupTableFSM::TransformGraph(const HyperGraph & graph) const {
    int VALID_NODE = 9999999; 
    HyperGraph* _graph = new HyperGraph;
    Sentence sent = graph.GetWords();
    _graph->SetWords(sent);

    HieroRuleSpans span = HieroRuleSpans();
    HieroNodeMap node_map = HieroNodeMap();
    EdgeList edge_list = EdgeList(); 
    // For each starting point
    for(int i = sent.size()-1; i >= 0; i--) {
        // Add a size 0 node for unknown words
        LookupTableFSM::FindNode(node_map, i, i+1, unk_symbol_);
        // For each grammar, add rules
        BOOST_FOREACH(RuleFSM* rule_fsm, rule_fsms_)
            rule_fsm->BuildHyperGraphComponent(node_map, edge_list, sent, "", i, span);
    }

    // Add rules for unknown words
    vector<TailSpanKey> temp_spans;
    BOOST_FOREACH(HieroNodeMap::value_type & val, node_map) {
        BOOST_FOREACH(HeadNodePairs::value_type & head_node, val.second) {
            HyperNode* node = head_node.second;
            pair<int,int> node_span = node->GetSpan();
            if (node_span.second - node_span.first == 1) {
                int i = node_span.first;
                if(node->GetEdges().size() == 0) {
                    TranslationRuleHiero* unk_rule = GetUnknownRule(sent[i], delete_unknown_? Dict::WID("") : sent[i], head_node.first);
                    HyperEdge* unk_edge = LookupTableFSM::TransformRuleIntoEdge(node_map,i,i+1,temp_spans,unk_rule,save_src_str_);
                    edge_list.push_back(unk_edge);
                    delete unk_rule;
                } 
            }
        }
    }

    // Find the root node
    HyperNode * root_node = NULL;
    HieroNodeMap::iterator big_span_node = node_map.find(make_pair(0,(int)sent.size()));
    if(big_span_node != node_map.end()) {
        HieroHeadLabels root_sym = GetRootSymbol();
        BOOST_FOREACH(HeadNodePairs::value_type & hnp, big_span_node->second) {
            if(hnp.first == root_sym) {
                root_node = hnp.second;
                break;
            }
        }
    }

    // If the node is not found, delete and return an empty graph
    if(root_node == NULL) {
        // cerr << "Could not find Span "<<Dict::WSym(GetRootSymbol()[0])<<"[0,"<<sent.size()<<"]"<<endl;
        BOOST_FOREACH (HyperEdge* edges, edge_list) 
            if(edges)
                delete edges;
        BOOST_FOREACH(HieroNodeMap::value_type nodes, node_map)
            BOOST_FOREACH(HeadNodePairs::value_type & hnp, nodes.second)
                delete hnp.second;
        return new HyperGraph;
    } else {
        // Deleting nodes that are unreachable from root node
        // First traverse the root node
        vector<HyperNode*> stack;
        stack.push_back(root_node);
        while (!stack.empty()) {
            HyperNode* now = stack.back();
            stack.pop_back();
            if (now->GetId() != VALID_NODE) {
                now->SetId(VALID_NODE); 
                BOOST_FOREACH(HyperEdge* edge, now->GetEdges()) {
                    edge->SetId(VALID_NODE);
                    BOOST_FOREACH(HyperNode* node, edge->GetTails()) {
                        stack.push_back(node);
                    }
                }
            }
        }
        // Delete the edges that are unreachable from root
        EdgeList::iterator it = edge_list.begin();
        while(it != edge_list.end()) {
            if ((*it)->GetId() != VALID_NODE) {
                delete *it;
                it = edge_list.erase(it);
            } else {
                ++it;
            }
        }

        // // Delete the nodes that are unreachable from root
        // HieroNodeMap::iterator itr = node_map.begin();
        // while(itr != node_map.end()) {
        //     if (itr->second->GetId() != 0) {
        //         node_map.erase(itr++);
        //     } else {
        //         ++itr;
        //     }
        // }
    }

    // Add the root node
    if (root_node != NULL) {
        root_node->SetId(-1);
        _graph->AddNode(root_node);
    }

    // Add the rest of the nodes
    BOOST_FOREACH (HieroNodeMap::value_type nodes, node_map) {
        BOOST_FOREACH(HeadNodePairs::value_type & head_node, nodes.second) {
            if(head_node.second->GetId() == VALID_NODE) {
                head_node.second->SetId(-1);
                _graph->AddNode(head_node.second);
            } else if (head_node.second != root_node) {
                delete head_node.second;
            }
        }
    }
    
    BOOST_FOREACH (HyperEdge* edges, edge_list) { 
        if(edges) {
            edges->SetId(-1);
            _graph->AddEdge(edges);
        } else {
            THROW_ERROR("All edges here should be valid, but found 1 with invalid.");
        }
    }

    return _graph;
}

void RuleFSM::BuildHyperGraphComponent(
        HieroNodeMap & node_map, 
        EdgeList & edge_list, 
        const Sentence & input,
        const string & state,
        int position, 
        HieroRuleSpans & spans) const {
    if (position >= (int)input.size())
        return;

    // First, match single words
    string next_state = state;
    next_state.append((char*)&input[position], sizeof(WordId));
    marisa::Agent agent;
    agent.set_query(next_state.c_str(), next_state.length());
    // Update the spans
    HieroRuleSpans rule_span_next = HieroRuleSpans(spans);
    pair<int,int> span = make_pair(position,position+1);
    rule_span_next.push_back(span);
    // cerr << " Searching word " <<  PrintState(state) << "->" << PrintState(next_state) << endl;
    if(trie_.predictive_search(agent)) {
        // cerr << " FOUND WORD " <<  PrintState(state) << "->" << PrintState(next_state) << endl;
        // If this exactly matched a rule add the rules
        marisa::Agent agent_exact;
        agent_exact.set_query(next_state.c_str(), next_state.length());
        if(trie_.lookup(agent_exact)) {
            BOOST_FOREACH(TranslationRuleHiero* rule, rules_[agent_exact.key().id()]) {
                // cerr << "Matched word for "<<PrintState(next_state)<<" == "<<PrintState(string(agent_exact.key().ptr(), agent_exact.key().length()))<<": " << *rule << endl;
                edge_list.push_back(LookupTableFSM::TransformRuleIntoEdge(rule, rule_span_next, node_map, save_src_str_));
            }
        }
        // Recurse to match the following rules
        BuildHyperGraphComponent(node_map, edge_list, input, next_state, position+1, rule_span_next);
    }

    // Continue until the end of the sentence or the max span length
    int until = min((int)input.size(), position+span_length_);
    for(int next_pos = position+1; next_pos <= until; next_pos++) {
        pair<int,int> span(position,next_pos);
        // If this is the root, ensure unary nodes are expanded
        if(state.length() == 0) {
            set<HieroHeadLabels> next_set;
            int last_size;
            do {
                last_size = next_set.size();
                BOOST_FOREACH(const UnaryMap::value_type & val, unaries_) {
                    HieroNodeMap::iterator it = node_map.find(span);
                    if(it != node_map.end()) {
                        if(it->second.find(val.first) != it->second.end()) {
                            BOOST_FOREACH(HieroHeadLabels child_lab, val.second) {
                                next_set.insert(child_lab);
                                LookupTableFSM::FindNode(node_map, position, next_pos, child_lab);
                            }
                        }
                    }
                }
            } while(last_size != (int)next_set.size());
        }
        // Find nodes that match the current span
        HieroNodeMap::iterator node_it = node_map.find(span);
        if(node_it == node_map.end()) continue;
        // Build every node
        HieroRuleSpans rule_span_next = HieroRuleSpans(spans);
        rule_span_next.push_back(span);
        BOOST_FOREACH(const HeadNodePairs::value_type & heads_node, node_it->second) {
            vector<WordId> sym(heads_node.first);
            BOOST_FOREACH(WordId & wid, sym)
                wid = -1-wid;
            string next_state = state;
            next_state.append((char*)&sym[0], sizeof(WordId)*sym.size());
            marisa::Agent agent;
            agent.set_query(next_state.c_str(), next_state.length());
            // cerr << " Searching nonterm " <<  PrintState(state) << "->" << PrintState(next_state) << endl;
            if(trie_.predictive_search(agent)) {
                // cerr << " FOUND NONTERM " <<  PrintState(state) << "->" << PrintState(next_state) << endl;
                // // TODO: Maybe we need to do something with unks?
                // if(span.second - span.first == 1 && node_it->second.find(sym) == node_it->second.end()) {
                //     cerr << "Adding unknown node @ " << span.first << ":" << span.second << ", " << sym << endl;
                //     LookupTableFSM::FindNode(node_map, span.first, span.second, sym);
                // }
                // If this exactly matched a rule add the rules
                marisa::Agent agent_exact;
                agent_exact.set_query(next_state.c_str(), next_state.length());
                if(trie_.lookup(agent_exact)) {
                    BOOST_FOREACH(TranslationRuleHiero* rule, rules_[agent_exact.key().id()]) {
                        // cerr << "Matched nonterm: " << *rule << endl;
                        edge_list.push_back(LookupTableFSM::TransformRuleIntoEdge(rule, rule_span_next, node_map, save_src_str_));
                    }
                }
                // Recurse to match the next node
                BuildHyperGraphComponent(node_map, edge_list, input, next_state, next_pos, rule_span_next);
            }
        }
    }

}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(TranslationRuleHiero* rule, const HieroRuleSpans & rule_span,
    HieroNodeMap & node_map, const bool save_src_str)
{
    vector<int> non_term_position = rule->GetSrcData().GetNontermPositions();
    vector<TailSpanKey > span_temp;
    for (int i=0 ; i < (int)non_term_position.size(); ++i) {
        // cerr << "non_term_position[" << i << "] == " << non_term_position[i] << endl;
        // cerr << "rule_span.size() " << rule_span.size() << endl;
        if(non_term_position[i] >= (int)rule_span.size())
            THROW_ERROR("non_term_position[" << i << "] "<<non_term_position[i]<<" >= " << rule_span.size());
        span_temp.push_back(make_pair(i,rule_span[non_term_position[i]]));
    }
    int head_first = rule_span[0].first;
    int head_second = rule_span[(int)rule_span.size()-1].second;
    return TransformRuleIntoEdge(node_map, head_first, head_second, span_temp, rule, save_src_str);
}

HyperEdge* LookupTableFSM::TransformRuleIntoEdge(HieroNodeMap& node_map, 
        const int head_first, const int head_second, const vector<TailSpanKey> & tail_spans, 
        TranslationRuleHiero* rule, const bool save_src_str)
{
    // // DEBUG start
    // cerr << " TransformRule @ " << head_first << "," << head_second << " ->";
    // BOOST_FOREACH(const TailSpanKey & tsk, tail_spans) {
    //     WordId symid = rule->GetSrcData().GetSym(tsk.first);
    //     cerr << " " << (symid >= 0 ? Dict::WSym(symid) : "NULL") << "---" << tsk.second.first << "," << tsk.second.second;
    // }
    // cerr << " ||| " << rule->GetSrcData() << endl;
    // // DEBUG end

    HyperEdge* hedge = new HyperEdge;
    HyperNode* head = FindNode(node_map, head_first, head_second, rule->GetHeadLabels());
    hedge->SetHead(head);
    if (save_src_str)
        hedge->SetSrcStr(Dict::PrintAnnotatedWords(rule->GetSrcData()));
    hedge->SetRule(rule);
    head->AddEdge(hedge);
    TailSpanKey tail_span;
    BOOST_FOREACH(tail_span, tail_spans) {
        HyperNode* tail = FindNode(node_map, tail_span.second.first, tail_span.second.second, rule->GetChildHeadLabels(tail_span.first));
        tail->SetSpan(tail_span.second);
        hedge->AddTail(tail);
        tail = NULL;
    }
    head = NULL;
    return hedge;
}

// Get an HyperNode, indexed by its span in some map.
HyperNode* LookupTableFSM::FindNode(HieroNodeMap& map_ptr, 
        const int span_begin, const int span_end, const HieroHeadLabels& head_label)
{
    if (span_begin < 0 || span_end < 0) 
        THROW_ERROR("Invalid span range in constructing HyperGraph.");
    pair<int,int> span = make_pair(span_begin,span_end);

    HieroNodeMap::iterator it = map_ptr.find(span);
    if (it == map_ptr.end()) {
        // Fresh New Node!
        HyperNode* ret = new HyperNode;
        ret->SetSpan(make_pair(span_begin,span_end));
        ret->SetSym(head_label[0]);
        map_ptr[span][head_label] = ret;
        // cerr << "Adding node! " << span.first << ":" << span.second << ", " << head_label << endl;
        return ret;
    } else {
        HeadNodePairs::iterator it2 = it->second.find(head_label);
        if (it2 == it->second.end()) {
            // Fresh New Node!
            HyperNode* ret = new HyperNode;
            ret->SetSpan(make_pair(span_begin,span_end));
            ret->SetSym(head_label[0]);
            map_ptr[span][head_label] = ret;
            // cerr << "Adding node! " << span.first << ":" << span.second << ", " << head_label << endl;
            return ret;
        } else {
            return it2->second;
        }
    }
}

TranslationRuleHiero* LookupTableFSM::GetUnknownRule(WordId unknown_word, const HieroHeadLabels& head_labels) {
    return GetUnknownRule(unknown_word,unknown_word, head_labels);
}

TranslationRuleHiero* LookupTableFSM::GetUnknownRule(WordId src, WordId unknown_word, const HieroHeadLabels& head_labels) 
{
    CfgDataVector target;
    for (int i=1; i < (int)head_labels.size(); ++i) 
        target.push_back(CfgData(Sentence(1,unknown_word),head_labels[i]));
    return new TranslationRuleHiero(
        target,
        Dict::ParseSparseVector("unk=1"),
        CfgData(Sentence(1, src), head_labels[0])
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

void LookupTableFSM::SetSpanLimits(const std::vector<int>& limits) {
    if(limits.size() != rule_fsms_.size())
        THROW_ERROR("The number of span limits (" << limits.size() << ") must be equal to the number of tm_files ("<<rule_fsms_.size()<<")");
    for(int i = 0; i < (int)limits.size(); i++)
        rule_fsms_[i]->SetSpanLimit(limits[i]);
}

void LookupTableFSM::SetSaveSrcStr(const bool save_src_str) {
    save_src_str_ = save_src_str;
    BOOST_FOREACH(RuleFSM* rfsm, rule_fsms_) 
        rfsm->SetSaveSrcStr(save_src_str);
}


///////////////////////////////////
///     LOOK UP NODE FSM         //
///////////////////////////////////
void LookupNodeFSM::AddEntry(const WordId & key, LookupNodeFSM* child_node) {
    lookup_map_[key] = child_node;
}

void LookupNodeFSM::AddNTEntry(const HieroHeadLabels& key, LookupNodeFSM* child_node) {
    nt_lookup_map_[key] = child_node;
}

void LookupNodeFSM::AddRule(TranslationRuleHiero* rule) {
    rules_.push_back(rule);
}

LookupNodeFSM* LookupNodeFSM::FindChildNode(const WordId key) const {
    LookupNodeMap::const_iterator it = lookup_map_.find(key); 
    return it != lookup_map_.end() ? it->second : NULL;
}

LookupNodeFSM* LookupNodeFSM::FindNTChildNode(const HieroHeadLabels& key) const {
    NTLookupNodeMap::const_iterator it = nt_lookup_map_.find(key);
    return it != nt_lookup_map_.end() ? it -> second : NULL;
}

void LookupNodeFSM::Print(std::ostream &out, WordId label, int indent, char prefix) const {
    float middle = lookup_map_.size() / 2;
    int i=0;
    char c_prefix = '/';
    BOOST_FOREACH(const LookupNodeMap::value_type &it, lookup_map_) {
        if (i++ == middle) {
            for (int j=0; j < indent; ++j) out << " ";
            out << prefix << Dict::WSym(label) << endl;
            c_prefix = '\\';
        }
        it.second->Print(out,it.first < 0 ? -it.first : it.first, indent+6, c_prefix);
    }
    out << endl; 
}

LookupNodeFSM::~LookupNodeFSM() { 
    BOOST_FOREACH(LookupNodeMap::value_type &it, lookup_map_) {
        delete it.second++;
    }
    BOOST_FOREACH(TranslationRuleHiero* rule, rules_) {
        delete rule;
    }
}
