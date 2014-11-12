
#include <travatar/translation-rule.h>
#include <travatar/lookup-table.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/global-debug.h>
#include <boost/foreach.hpp>
#include <queue>

using namespace travatar;
using namespace boost;
using namespace std;

LookupTable::LookupTable() : 
    unk_rule_(CfgDataVector(), Dict::ParseSparseVector("unk=1")),
    match_all_unk_(false), save_src_str_(false), consider_trg_(false) { }

LookupTable::~LookupTable() { }

// Find all the translation rules rooted at a particular node in a parse graph
vector<boost::shared_ptr<LookupState> > LookupTable::LookupSrc(
            const HyperNode & node, 
            const vector<boost::shared_ptr<LookupState> > & old_states) const {
    vector<boost::shared_ptr<LookupState> > ret_states;
    BOOST_FOREACH(const boost::shared_ptr<LookupState> & state, old_states) {
        // Match the current node
        boost::shared_ptr<LookupState> my_state(MatchNode(node, *state));
        if(my_state != NULL) ret_states.push_back(my_state);
        // Match all rules the require descent into the next node
        boost::shared_ptr<LookupState> start_state(MatchStart(node, *state));
        if(start_state == NULL) continue;
        // Cycle through all the edges from this node
        BOOST_FOREACH(const HyperEdge * edge, node.GetEdges()) {
            vector<boost::shared_ptr<LookupState> > my_states(1, start_state);
            // Cycle through all the tails in this edge
            BOOST_FOREACH(const HyperNode * child, edge->GetTails())
                my_states = LookupSrc(*child, my_states);
            // Finish all the states found
            BOOST_FOREACH(const boost::shared_ptr<LookupState> & my_state, my_states) {
                boost::shared_ptr<LookupState> fin_state(MatchEnd(node, *my_state));
                if(fin_state != NULL) {
                    fin_state->AddFeatures(edge->GetFeatures());
                    ret_states.push_back(fin_state);
                }
            }
        }
        
    }
    return ret_states;
}

HyperGraph * LookupTable::TransformGraph(const HyperGraph & parse) const {
    if (consider_trg_) {
        return TransformGraphSrcTrg(parse);
    } else {
        return TransformGraphSrc(parse);
    }
}

HyperGraph * LookupTable::TransformGraphSrc(const HyperGraph & parse) const {
    // First, for each non-terminal in the input graph, make a node in the output graph
    // and lookup the rules matching each node
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(parse.GetWords());
    std::map<int,int> node_map, rev_node_map;
    vector<vector<boost::shared_ptr<LookupState> > > lookups;
    vector<boost::shared_ptr<LookupState> > init_state;
    init_state.push_back(boost::shared_ptr<LookupState>(GetInitialState()));
    BOOST_FOREACH(const HyperNode * node, parse.GetNodes()) {
        if(!node->IsTerminal()) {
            rev_node_map.insert(make_pair(node_map.size(), node->GetId()));
            node_map.insert(make_pair(node->GetId(), node_map.size()));
            HyperNode * next_node = new HyperNode();
            ret->AddNode(next_node);
            next_node->SetSym(node->GetSym());
            next_node->SetSpan(node->GetSpan());
            lookups.push_back(LookupSrc(*node, init_state));
        }
    }
    // For each node
    BOOST_FOREACH(HyperNode * next_node, ret->GetNodes()) {
        // If there is at least one matching node
        int my_size = lookups[next_node->GetId()].size();
        if(my_size > 0) {
            // For each matched portion of the source tree
            BOOST_FOREACH(const boost::shared_ptr<LookupState> & state, lookups[next_node->GetId()]) {
                // For each tail in the 
                vector<HyperNode*> next_tails;
                BOOST_FOREACH(const HyperNode * tail, state->GetNonterms())
                    next_tails.push_back(ret->GetNode(node_map[tail->GetId()]));
                // For each rule
                BOOST_FOREACH(const TranslationRule * rule, *FindRules(*state)) {
                    HyperEdge * next_edge = new HyperEdge(next_node);
                    next_edge->SetTails(next_tails);
                    next_edge->SetRule(rule, state->GetFeatures());
                    if(save_src_str_) next_edge->SetSrcStr(state->GetString());
                    next_node->AddEdge(next_edge);
                    ret->AddEdge(next_edge);
                }
            }
        }
        // For unmatched nodes, add an unknown rule for every edge in the parse
        if(my_size == 0 || match_all_unk_) {
            const HyperNode * parse_node = parse.GetNode(rev_node_map[next_node->GetId()]);
            BOOST_FOREACH(const HyperEdge * parse_edge, parse_node->GetEdges()) {
                ostringstream oss;
                if(save_src_str_) oss << Dict::WSym(parse_node->GetSym()) << " (";
                HyperEdge * next_edge = new HyperEdge(next_node);
                BOOST_FOREACH(const HyperNode * parse_node, parse_edge->GetTails())
                    if(!parse_node->IsTerminal())
                        next_edge->AddTail(ret->GetNode(node_map[parse_node->GetId()]));
                next_edge->SetRule(GetUnknownRule(), parse_edge->GetFeatures());
                if(next_edge->GetTails().size() > 0) {
                    vector<int> trg(next_edge->GetTails().size());
                    for(int i = 0; i < (int)trg.size(); i++) {
                        trg[i] = -1 - i;
                        if(save_src_str_) oss << " x" << i << ':' << Dict::WSym(next_edge->GetTail(i)->GetSym());
                    }
                    next_edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors, trg));
                } else {
                    pair<int,int> span = parse_node->GetSpan();
                    if(span.second - span.first != 1)
                        THROW_ERROR("Multi-terminal edges are not supported.");
                    vector<WordId> trg_words(1, parse.GetWord(span.first));
                    if(save_src_str_) oss << " \"" << Dict::WSym(parse.GetWord(span.first)) << '"';
                    next_edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors, trg_words));
                }
                next_node->AddEdge(next_edge);
                if(save_src_str_) {
                    oss << " )";
                    next_edge->SetSrcStr(oss.str());
                }
                ret->AddEdge(next_edge);
            }
        }
    }
    return ret;
}

HyperGraph * LookupTable::TransformGraphSrcTrg(const HyperGraph & parse) const {
    typedef map<vector<WordId>, HyperNode*> TargetMap;
    typedef map<int, TargetMap> NodeMap;
    typedef map<int, HyperNode*> GeneralMap;
    // First, for each non-terminal in the input graph, make a node in the output graph
    // and lookup the rules matching each node
    HyperGraph * ret = new HyperGraph;
    ret->SetWords(parse.GetWords());
    
    priority_queue<SpannedState,vector<SpannedState>,SpannedStateComparator> lookups;
    vector<boost::shared_ptr<LookupState> > init_state;
    init_state.push_back(boost::shared_ptr<LookupState>(GetInitialState()));
    BOOST_REVERSE_FOREACH(const HyperNode * node, parse.GetNodes()) {
        if(!node->IsTerminal()) {
            lookups.push(make_pair(LookupSrc(*node, init_state),node));
        } 
    }

    NodeMap node_map;
    GeneralMap general_map;
    // Starting from the shorter span (construct the hypergraph from bottom to top)
    while (!lookups.empty()) {
        // Next State
        const SpannedState spanned_state = lookups.top();
        lookups.pop();
        const HyperNode* input_node = spanned_state.second;
        // Expand This state
        TargetMap indexed_head;
        BOOST_FOREACH (const boost::shared_ptr<LookupState> & state, spanned_state.first) {
            std::vector<const HyperNode*> non_terms = state->GetNonterms();
            BOOST_FOREACH (const TranslationRule * rule, *FindRules(*state)) {
                CfgDataVector trg_data = rule->GetTrgData();
                // head of this rule
                vector<WordId> heads(trg_data.size(),0);
                for (size_t i = 0; i < trg_data.size(); ++i) {
                    heads[i] = trg_data[i].label;
                }
                // whether there exists this head before
                TargetMap::const_iterator it = indexed_head.find(heads);
                HyperNode * head_node; 
                if (it == indexed_head.end()) {
                    head_node = new HyperNode();
                    head_node->SetSym(input_node->GetSym());
                    head_node->SetSpan(input_node->GetSpan());
                } else {
                    head_node = it->second;
                }
                // attempt to apply the rule
                HyperEdge * next_edge = new HyperEdge(head_node);
                unsigned int penalty = 0;
                bool child_ok = true;
                for (size_t i=0; child_ok && i < trg_data[0].GetSymSize(); ++i) {
                    // the head of the child's nt
                    vector<WordId> child_head(trg_data.size(),0);
                    for (size_t j=0; j < trg_data.size(); ++j) {
                        child_head[j] = (trg_data[j].GetSym(i));
                    }
                    // attempt to find it 
                    TargetMap map = node_map[non_terms[i]->GetId()];
                    it = map.find(child_head);
                    if (it == map.end()) {
                        GeneralMap::const_iterator it = general_map.find(non_terms[i]->GetId());
                        // OK, this part is connecting the rule regardless the target head,
                        // just connecting to the child, adding penalty
                        if (it != general_map.end()) {
                            ++penalty;
                            next_edge->AddTail(it->second);
                        } else {
                            child_ok = false;
                        }
                    } else {
                        // if match, add it to the tail
                        next_edge->AddTail(it->second);
                    }
                }
                // check if all nt can be found so the rule is legal
                if (child_ok) {
                    SparseVector feat = state->GetFeatures();
                    if (penalty > 0) feat.Add("unmatch_trg", penalty);
                    next_edge->SetRule(rule, feat);
                    if (save_src_str_) next_edge->SetSrcStr(state->GetString());
                    head_node->AddEdge(next_edge);
                    ret->AddEdge(next_edge);
                    indexed_head.insert(make_pair(heads, head_node));
                } else {
                    // this rule can't be applied, delete the component immediately
                    delete next_edge;
                    delete head_node;
                }
            } // for rule
        } // for state
        node_map.insert(make_pair(input_node->GetId(),indexed_head));
        // Connect all the factored state into parse state with no cost!
        // This parse state is built according to input tree
        HyperNode* parse_node = new HyperNode();
        parse_node->SetSym(Dict::WID("χ"));
        parse_node->SetSpan(input_node->GetSpan());
        BOOST_FOREACH(TargetMap::value_type val, indexed_head) {
            HyperEdge* connecting_edge = new HyperEdge(parse_node);
            ostringstream oss;
            vector<WordId> trg_words(1, -1);
            if (save_src_str_) oss << Dict::WSym(input_node->GetSym()) << " ( x0:" << Dict::WSym(parse_node->GetSym()) << " )";
            CfgDataVector trg_data(GlobalVars::trg_factors, trg_words);
            connecting_edge->AddTail(val.second);
            connecting_edge->SetTrgData(trg_data);
            if (save_src_str_) connecting_edge->SetSrcStr(oss.str());
            parse_node->AddEdge(connecting_edge);
            ret->AddEdge(connecting_edge);
        }
        // Whether or not, connecting the parse node to its child 
        // according to input tree
        // it we match all unk, connect the parse state regardlessly
        if (indexed_head.size() == (size_t) 0 || match_all_unk_) {
            // Adding the tail to other parse state
            if (input_node->IsPreTerminal()) {
                // Translating the node to itself
                HyperEdge* idem = new HyperEdge(parse_node);
                WordId word = parse.GetWord(input_node->GetSpan().first);
                ostringstream oss;
                if (save_src_str_) oss << Dict::WSym(input_node->GetSym()) << " ( \"" << Dict::WSym(word) << "\" )";
                vector<WordId> trg_words(1, word);
                CfgDataVector trg_data(GlobalVars::trg_factors, trg_words);
                idem->SetRule(GetUnknownRule(), input_node->GetEdge(0)->GetFeatures());
                idem->SetTrgData(trg_data);
                if (save_src_str_) idem->SetSrcStr(oss.str());
                parse_node->AddEdge(idem);
                ret->AddEdge(idem);
            } else {
                BOOST_FOREACH(HyperEdge* edge, input_node->GetEdges()) {
                    HyperEdge * parse_edge = new HyperEdge(parse_node);
                    ostringstream oss;
                    if (save_src_str_) oss << Dict::WSym(input_node->GetSym()) << " ("; 
                    vector<WordId> trg(edge->GetTails().size());
                    for (size_t i=0; i < trg.size(); ++i) {
                        if(save_src_str_) oss << " x" << i << ':' << Dict::WSym(edge->GetTail(i)->GetSym()); 
                        trg[i] = -1 - i;
                    }
                    parse_edge->SetRule(GetUnknownRule(), edge->GetFeatures());
                    parse_edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors,trg));
                    BOOST_FOREACH(HyperNode* tail, edge->GetTails()) {
                        HyperNode* child = general_map.find(tail->GetId())->second;
                        parse_edge->AddTail(child);
                    }
                    if (save_src_str_) {
                        oss << " )";
                        parse_edge->SetSrcStr(oss.str());
                    }
                    parse_node->AddEdge(parse_edge);
                    ret->AddEdge(parse_edge);
                }
            }
        }
        general_map.insert(make_pair(input_node->GetId(), parse_node));
    }
    BOOST_FOREACH(GeneralMap::value_type val, general_map) {
        ret->AddNode(val.second);
    }
    BOOST_FOREACH(const NodeMap::value_type node_map_v, node_map) {
        BOOST_FOREACH(TargetMap::value_type node, node_map_v.second) {
            ret->AddNode(node.second);
        }
    }
    
    return ret;
}


const TranslationRule * LookupTable::GetUnknownRule() const { return &unk_rule_; }

bool SpannedStateComparator::operator() (const SpannedState& lhs, const SpannedState& rhs) const {
    pair<int,int> p1 = lhs.second->GetSpan();
    pair<int,int> p2 = rhs.second->GetSpan();
    int result = (p1.second - p1.first) - (p2.second - p2.first);
    if (result == 0)  
        result = (p1.second - p2.second) > 0;
    return result > 0;
}
