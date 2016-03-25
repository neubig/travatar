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
                   empty_symbol_(HieroHeadLabels(vector<WordId>(GlobalVars::trg_factors+1,Dict::WID("")))),
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

HyperEdge * LookupTableFSM::LookupUnknownRule(int index, const Sentence & sent, const HieroHeadLabels & syms, HieroNodeMap & node_map) const {
    TranslationRuleHiero* unk_rule = GetUnknownRule(sent[index], delete_unknown_? Dict::WID("") : sent[index], syms);
    vector<TailSpanKey> temp_spans;
    HyperEdge* unk_edge = RuleFSM::TransformRuleIntoEdge(node_map, index, index+1, temp_spans, unk_rule, save_src_str_);
    delete unk_rule;
    return unk_edge;
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

HyperGraph * LookupTableFSM::TransformGraph(const HyperGraph & graph) const {
    int VALID_NODE = 9999999; 
    HyperGraph* ret = new HyperGraph;
    Sentence sent = graph.GetWords();
    ret->SetWords(sent);

    HieroRuleSpans span = HieroRuleSpans();
    HieroNodeMap node_map = HieroNodeMap();
    EdgeList edge_list = EdgeList(); 

    // For each starting point
    for(int i = sent.size()-1; i >= 0; i--) {
        if (unk_symbol_ != empty_symbol_) {
            // Add a size 0 node for unknown words
            RuleFSM::FindNode(node_map, i, i+1, unk_symbol_);
        }
        // For each grammar, add rules
        BOOST_FOREACH(RuleFSM* rule_fsm, rule_fsms_)
            rule_fsm->BuildHyperGraphComponent(node_map, edge_list, sent, "", i, span);
    }

    // Add rules for unknown words
    BOOST_FOREACH(HieroNodeMap::value_type & val, node_map) {
        BOOST_FOREACH(HeadNodePairs::value_type & head_node, val.second) {
            HyperNode* node = head_node.second;
            HieroHeadLabels node_label = head_node.first;
            pair<int,int> node_span = node->GetSpan();
            // -unk_symbol is the empty string, 
            // we add unk rule to all symbol
            // OR if -unk_symbol is not the empty string we add unk rule only to SPECIFIED symbol. 
            if (unk_symbol_ == empty_symbol_ || unk_symbol_ == node_label) {
                // Unknown spanning [x, x+1]
                if (node_span.second - node_span.first == 1) {
                    if(node->GetEdges().size() == 0) {
                        HyperEdge * unk_edge = LookupUnknownRule(node_span.first, sent, head_node.first, node_map);
                        edge_list.push_back(unk_edge);
                    } 
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
        ret->AddNode(root_node);
    }

    // Add the rest of the nodes
    BOOST_FOREACH (HieroNodeMap::value_type nodes, node_map) {
        BOOST_FOREACH(HeadNodePairs::value_type & head_node, nodes.second) {
            if(head_node.second->GetId() == VALID_NODE) {
                head_node.second->SetId(-1);
                ret->AddNode(head_node.second);
            } else if (head_node.second != root_node) {
                delete head_node.second;
            }
        }
    }
    
    BOOST_FOREACH (HyperEdge* edges, edge_list) { 
        if(edges) {
            edges->SetId(-1);
            ret->AddEdge(edges);
        } else {
            THROW_ERROR("All edges here should be valid, but found 1 with invalid.");
        }
    }

    return ret;
}

