#include <travatar/rule-fsm.h>
#include <travatar/string-util.h>
#include <travatar/global-debug.h>
#include <travatar/dict.h>
#include <travatar/translation-rule-hiero.h>
#include <travatar/lookup-table-cfglm.h>
#include <travatar/hyper-graph.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

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
        if(!src_data.NontermsAreOrdered())
            THROW_ERROR("Nonterminal IDs on the source side must be in ascending order, but are not at line: " << endl << line);
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
            BOOST_FOREACH(const HieroHeadLabels & target, val.second) {
                if(val.first == target)
                    THROW_ERROR("Unary cycles are not allowed in CFG grammars, but found one with for label " << Dict::WSym(target[0]) << endl);
                UnaryMap::iterator it = unaries.find(target);
                if(it != unaries.end()) {
                    BOOST_FOREACH(const HieroHeadLabels & second_trg, it->second) {
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
        // cerr << "Adding agent: " << CFGPath::PrintAgent(agent) << endl;
        if(!ret->GetTrie().lookup(agent))
            THROW_ERROR("Internal error when building rule table");
        main_rules[agent.key().id()].swap(rule.second);
        // BOOST_FOREACH(TranslationRuleHiero * hier, main_rules[agent.key().id()])
        //     cerr << "RULE: " << PrintState(rule.first) << " @ "<<agent.key().id()<<": " << *hier << endl;
    }
    return ret;
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
                edge_list.push_back(RuleFSM::TransformRuleIntoEdge(rule, rule_span_next, node_map, save_src_str_));
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
                                RuleFSM::FindNode(node_map, position, next_pos, child_lab);
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
                //     RuleFSM::FindNode(node_map, span.first, span.second, sym);
                // }
                // If this exactly matched a rule add the rules
                marisa::Agent agent_exact;
                agent_exact.set_query(next_state.c_str(), next_state.length());
                if(trie_.lookup(agent_exact)) {
                    BOOST_FOREACH(TranslationRuleHiero* rule, rules_[agent_exact.key().id()]) {
                        // cerr << "Matched nonterm: " << *rule << endl;
                        edge_list.push_back(RuleFSM::TransformRuleIntoEdge(rule, rule_span_next, node_map, save_src_str_));
                    }
                }
                // Recurse to match the next node
                BuildHyperGraphComponent(node_map, edge_list, input, next_state, next_pos, rule_span_next);
            }
        }
    }

}

HyperEdge* RuleFSM::TransformRuleIntoEdge(TranslationRuleHiero* rule, const HieroRuleSpans & rule_span,
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

HyperEdge* RuleFSM::TransformRuleIntoEdge(HieroNodeMap& node_map, 
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
HyperNode* RuleFSM::FindNode(HieroNodeMap& map_ptr, 
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
