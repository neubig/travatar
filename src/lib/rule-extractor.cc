#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/rule-extractor.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/hiero-rule-table.h>
#include <queue>
#include <stack>
#include <list>
#include <map>

using namespace travatar;
using namespace std;
using namespace boost;

// Create a rule graph using Mi and Huang's forest-based rule extraction algorithm
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
HyperGraph* ForestExtractor::ExtractMinimalRules(
        HyperGraph & src_parse,
        const Alignment & align) const {
    // Calculate spans and the frontier set (GHKM)
    src_parse.CalculateFrontiers(align.GetSrcAlignments());
    // Contains a rule and the as-of-yet-unexpanded nodes
    typedef pair<HyperEdge*,deque<HyperNode*> > FragFront;
    // First, build a graph of nodes that contain the same spans as the original nodes
    HyperGraph* ret = new HyperGraph;
    ret->SetWords(src_parse.GetWords());
    map<int,int> old_new_ids;
    BOOST_FOREACH(HyperNode * v, src_parse.GetNodes()) {
        if(v->IsFrontier() == HyperNode::IS_FRONTIER) {
            old_new_ids.insert(make_pair(v->GetId(), ret->NumNodes()));
            HyperNode* node = new HyperNode(v->GetSym(), -1, v->GetSpan(), ret->NumNodes());
            node->SetTrgSpan(v->GetTrgSpan());
            ret->AddNode(node);
        }
    }
    // Create the rules
    // See Mi and Huang Algorithm 1 for pseudo-code
    BOOST_FOREACH(HyperNode * v, src_parse.GetNodes()) {
        // Skip non-frontier nodes
        if(v->IsFrontier() != HyperNode::IS_FRONTIER) continue;
        HyperNode* new_node = ret->GetNode(old_new_ids[v->GetId()]);
        // Create the queue of nodes to process
        stack<FragFront> open;
        HyperEdge* frag = new HyperEdge(new_node);
        deque<HyperNode*> front; front.push_back(v);
        open.push(FragFront(frag, front));
        // Continue processing until the queue is empty
        while(open.size() != 0) {
            FragFront frag_front = open.top(); open.pop();
            // If there are no more nodes to expand, add the rule
            if(frag_front.second.size() == 0) {
                ret->AddEdge(frag_front.first);
                new_node->AddEdge(frag_front.first);
            }
            // Otherwise, expand the node on the frontier
            else {
                // Get the next node to process and remove it from the stack
                HyperNode* u = frag_front.second.front(); frag_front.second.pop_front();
                // If it has no edges, push back onto the stack and continue
                if(u->GetEdges().size() == 0) {
                    open.push(frag_front);
                    continue;
                }
                // Otherwise process all hyperedges of u
                BOOST_FOREACH(HyperEdge* e, u->GetEdges()) {
                    // Calculate the nodes that still need to be expanded
                    // We want to push these on the stack backwards so we can process
                    // in ascending order
                    deque<HyperNode*> new_front;
                    HyperEdge * new_frag(new HyperEdge(*frag_front.first));
                    BOOST_FOREACH(HyperNode* t, e->GetTails()) {
                        // If this is not a frontier node, push it on the queue
                        if(t->IsFrontier() != HyperNode::IS_FRONTIER)
                            new_front.push_back(t);
                        else
                            new_frag->AddTail(ret->GetNode(old_new_ids[t->GetId()]));
                    }
                    BOOST_FOREACH(HyperNode * stack_node, frag_front.second)
                        new_front.push_back(stack_node);
                    // Push this on to the queue
                    new_frag->AddFragmentEdge(e);
                    open.push(FragFront(new_frag, new_front));
                }
                // Delete the partial fragment
                delete frag_front.first;
            }
        }
    }
    return ret;
}

HyperGraph * ForestExtractor::AttachNullsTop(const HyperGraph & rule_graph,
                                           const Alignment & align,
                                           int trg_len) {
    HyperGraph * ret = new HyperGraph(rule_graph);
    if(ret->NumNodes() > 0) {
        vector<bool> nulls(trg_len, true);
        BOOST_FOREACH(const Alignment::AlignmentPair & a, align.GetAlignmentVector())
            nulls[a.second] = false;
        AttachNullsTop(nulls, *ret->GetNode(0));
    }
    return ret;
}

void ForestExtractor::AttachNullsTop(vector<bool> & nulls,
                                     HyperNode & node) {
    pair<int,int> trg_covered = node.GetTrgCovered();
    if(trg_covered.first == -1) return;
    trg_covered.second = min(trg_covered.second, (int)nulls.size());
    vector<bool> child_covered(trg_covered.second-trg_covered.first, false);
    BOOST_FOREACH(HyperNode * tail, (node.GetEdges()[0])->GetTails()) {
        pair<int,int> tail_cov = tail->GetTrgCovered();
        for(int i = tail_cov.first; i < tail_cov.second; i++)
            child_covered[i-trg_covered.first] = true;
        AttachNullsTop(nulls, *tail);
    }
    for(int i = 0; i < (int)child_covered.size(); i++) {
        if(!child_covered[i]) {
            nulls[i+trg_covered.first] = true;
            node.GetTrgSpan().insert(i+trg_covered.first);
        }
    }
}


HyperGraph * ForestExtractor::AttachNullsExhaustive(
                                           const HyperGraph & rule_graph,
                                           const Alignment & align,
                                           int trg_len) {
    HyperGraph * ret = new HyperGraph(rule_graph);
    if(rule_graph.NumNodes() == 0) return ret;
    ret->DeleteNodes();
    ret->DeleteEdges();
    ret->AddNode(new HyperNode);
    ret->GetNode(0)->SetFrontier(HyperNode::NOT_FRONTIER);
    vector<bool> nulls(trg_len, true);
    // Find the null aligned target words
    BOOST_FOREACH(const Alignment::AlignmentPair &a, align.GetAlignmentVector())
        nulls[a.second] = false;
    vector< SpanNodeVector > new_nodes(rule_graph.NumNodes());
    // Get the expanded nodes in the new graph 
    GetExpandedNodes(nulls, *rule_graph.GetNode(0), new_nodes, max_attach_);
    // Add the links from the pseudo-node to the root
    BOOST_FOREACH( SpanNodeVector::value_type & val, new_nodes[0] ) {
        HyperEdge * edge = new HyperEdge(ret->GetNode(0));
        edge->AddTail(val.second);
        ret->GetNode(0)->AddEdge(edge);
        ret->AddEdge(edge);
    }
    // Add these to the graph
    BOOST_FOREACH( SpanNodeVector & vec, new_nodes ) {
        BOOST_FOREACH( SpanNodeVector::value_type & val, vec ) {
            ret->AddNode(val.second);
            BOOST_FOREACH( HyperEdge * edge, val.second->GetEdges() ) {
                ret->AddEdge(edge);
            }
        }
    }
    return ret;
}

// For each old_node
// Return a vector of new nodes that have been expanded to cover the surrounding nulls
// Where each node is also annotated with the set of surrounding positions it covers (out of all the positions possible)
const ForestExtractor::SpanNodeVector & ForestExtractor::GetExpandedNodes(
                            const vector<bool> & nulls,
                            const HyperNode & old_node,
                            vector<ForestExtractor::SpanNodeVector> & expanded,
                            int my_attach
                ) {
    int old_id = old_node.GetId();
    if(expanded[old_id].size())
        return expanded[old_id];
    // Expand the node itself
    expanded[old_id] = ExpandNode(nulls, old_node, my_attach);
    // Create the stack of partially processed tails
    typedef boost::tuple<set<int>, const HyperEdge*, HyperEdge*, int, int > q_tuple;
    queue< q_tuple > open_queue;
    // If the node has edges, expand the edges for each node
    BOOST_FOREACH(const SpanNodeVector::value_type & val, expanded[old_id]) {
        BOOST_FOREACH(const HyperEdge* edge, old_node.GetEdges()) {
            HyperEdge * new_edge = new HyperEdge(*edge);
            new_edge->SetHead(val.second);
            new_edge->SetId(-1);
            new_edge->GetTails().resize(0);
            open_queue.push(q_tuple(set<int>(), edge, new_edge, *val.second->GetTrgSpan().begin(), *val.second->GetTrgSpan().rbegin()));
        }
    }
    // While there are still edges we haven't finished, iterate
    while(open_queue.size() > 0) {
        q_tuple trip = open_queue.front();
        open_queue.pop();
        // If we are done, delete
        if(trip.get<1>()->NumTails() == trip.get<2>()->NumTails()) {
            trip.get<2>()->GetHead()->AddEdge(trip.get<2>());
        // Otherwise, expand the next node in the old edge
        } else {
            // cerr << "q@" <<old_id << " == " << open_queue.size() << " tails == " << trip.get<2>()->NumTails() << " / " << trip.get<1>()->NumTails() << endl;
            // Get the expanded nodes, but only expand if we don't have too many non-terminals
            // to prevent combinatorial explosion
            const SpanNodeVector & next_exp = GetExpandedNodes(nulls, 
                                                               *trip.get<1>()->GetTail(trip.get<2>()->NumTails()),
                                                               expanded,
                                                               (trip.get<1>()->NumTails() > max_nonterm_ ? 0 : max_attach_));
            // For all the possibilities
            BOOST_FOREACH(const SpanNodeVector::value_type & val, next_exp) {
                set<int> new_set = trip.get<0>();
                bool ok = true;
                // For each value covered by the tail, check to make sure that
                // we do not have any doubly covered nulls
                BOOST_FOREACH(int covered, val.first) {
                    ok = (new_set.find(covered) == new_set.end()) &&
                         covered >= trip.get<3>() &&
                         covered <= trip.get<4>();
                    if(!ok) break;
                    new_set.insert(covered);
                }
                // If we are OK, create a new edge and add it
                if(ok) {
                    HyperEdge * next_edge = new HyperEdge(*trip.get<2>());
                    next_edge->SetId(-1);
                    next_edge->AddTail(val.second);
                    open_queue.push(q_tuple(new_set, trip.get<1>(), next_edge, trip.get<3>(), trip.get<4>()));
                }
            }
            delete trip.get<2>();
        }
    }
    return expanded[old_id];
}

// Check to make sure that we can expand the node properly
ForestExtractor::SpanNodeVector ForestExtractor::ExpandNode(
                const vector<bool> & nulls,
                const HyperNode & old_node,
                int my_attach) const {
    SpanNodeVector ret;
    const set<int> & trg_span = old_node.GetTrgSpan();
    // For nodes that are entirely unaligned
    if(trg_span.size() == 0) {
        HyperNode * next_node = new HyperNode(old_node);
        next_node->SetId(-1);
        ret.push_back(make_pair(set<int>(), next_node));
        return ret;
    }
    // i_node stores the new node with all nulls before
    set<int> i_set;
    for(int i = *trg_span.begin(); i >= max(0, *trg_span.begin() - my_attach)  && (i == *trg_span.begin() || nulls[i]); i--) {
        if(nulls[i]) i_set.insert(i);
        set<int> j_set = i_set;
        for(int j = *trg_span.rbegin();
              j < min((int)nulls.size(), *trg_span.rbegin()+1+my_attach)  && 
              (j == *trg_span.rbegin() || nulls[j]); 
              j++) {
            if(nulls[j]) j_set.insert(j);
            HyperNode * j_node = new HyperNode(old_node);
            j_node->GetEdges().resize(0);
            j_node->SetId(-1);
            BOOST_FOREACH(int k, j_set)
                j_node->GetTrgSpan().insert(k);
            ret.push_back(make_pair(j_set, j_node));
        }
    }
    return ret;
}

void RuleExtractor::PrintRuleSurface(const HyperNode & node,
                                     const Sentence & src_sent,
                                     list<HyperEdge*> & remaining_fragments,
                                     int & tail_num,
                                     ostream & oss) const {
    // If this is a terminal, print its surface form
    if(node.IsTerminal()) {
        oss << "\"" << Dict::WSym(node.GetSym()) << '"';
        return;
    // If this is a frontier node
    } else if ((remaining_fragments.size() == 0) || (*remaining_fragments.begin())->GetHead()->GetId() != node.GetId()) {
        oss << "x" << tail_num++ << ':' << Dict::WSym(node.GetSym());
        return;
    }
    // If this is a non-terminal that is at the top of the tree
    oss << Dict::WSym(node.GetSym()) << " (";
    HyperEdge * my_edge = *remaining_fragments.begin();
    if(remaining_fragments.size() == 0)
        THROW_ERROR("Attempting to pop an empty stack");
    remaining_fragments.pop_front();
    BOOST_FOREACH(HyperNode* my_node, my_edge->GetTails()) {
        oss << ' ';
        PrintRuleSurface(*my_node, src_sent, remaining_fragments, tail_num,oss);
    }
    oss << " )";
}

// A utility function to get the span labele if it exists, and nothing if not
inline WordId GetSpanLabel(const LabeledSpans & trg_labs, const pair<int,int> & trg_span) {
    LabeledSpans::const_iterator it = trg_labs.find(trg_span);
    return (it != trg_labs.end()) ? it->second : -1;
}

// Creating a rule
string RuleExtractor::RuleToString(const HyperEdge & rule, const Sentence & src_sent, const Sentence & trg_sent, const LabeledSpans * trg_labs) const {
    // Get the target span for the top node
    const std::set<int> & trg_span = rule.GetHead()->GetTrgSpan();
    if(trg_span.size() == 0)
        THROW_ERROR("Empty target span in rule " << rule << endl << Dict::PrintWords(src_sent) << endl << Dict::PrintWords(trg_sent));
    int trg_begin = *trg_span.begin(), trg_end = *trg_span.rbegin()+1;
    int src_begin = rule.GetHead()->GetSpan().first, src_end = rule.GetHead()->GetSpan().second;
    // Create the covered target vector. Initially all are -1, indicating that
    // they have not yet been covered by a child frontier node
    vector<int> trg_cover(trg_end-trg_begin, -1), src_cover(src_end-src_begin, -1);
    const vector<HyperNode*> & tails = rule.GetTails();
    map<int,int> tail_map;
    // Tail labels
    vector<WordId> tail_labs;
    if(trg_labs) tail_labs.push_back(GetSpanLabel(*trg_labs, make_pair(trg_begin, trg_end)));
    // Handle all tails
    for(int i = 0; i < (int)tails.size(); i++) {
        tail_map[tails[i]->GetId()] = i;
        // Get the target and source span
        const std::set<int> & my_trg_span = tails[i]->GetTrgSpan();
        int my_trg_begin = *my_trg_span.begin(), my_trg_end = *my_trg_span.rbegin()+1;
        const pair<int,int> & my_src_span = tails[i]->GetSpan();
        // Add the target label
        if(trg_labs) tail_labs.push_back(GetSpanLabel(*trg_labs, make_pair(my_trg_begin, my_trg_end)));
        // Add the covers
        for(int j = my_trg_begin; j < my_trg_end; j++) {
            // Rules should never cover the same span
            if(trg_cover[j-trg_begin] != -1) {
                cerr << "src: " << Dict::PrintWords(src_sent) << endl << "trg: " << Dict::PrintWords(trg_sent) << endl << rule << endl;
                BOOST_FOREACH(HyperNode* tail, tails)
                    cerr << " " << *tail << endl;
                THROW_ERROR("Span covered by two target rules");
            }
            trg_cover[j-trg_begin] = i;
        }
        for(int j = my_src_span.first; j < my_src_span.second; j++) {
            // Rules should never cover the same span
            if(src_cover[j-src_begin] != -1) {
                cerr << "src: " << Dict::PrintWords(src_sent) << endl << "src: " << Dict::PrintWords(src_sent) << endl << rule << endl;
                BOOST_FOREACH(HyperNode* tail, tails)
                    cerr << " " << *tail << endl;
                THROW_ERROR("Span covered by two source rules");
            }
            src_cover[j-src_begin] = i;
        }
    }
    // Create the string to return
    ostringstream oss;
    // Traverse the rules in order of edge
    list<HyperEdge*> remaining_fragments;
    BOOST_FOREACH(HyperEdge *edge, rule.GetFragmentEdges()) {
        remaining_fragments.push_back(edge);
    }
    int tail_num = 0;
    PrintRuleSurface(*(*remaining_fragments.begin())->GetHead(), src_sent, remaining_fragments, tail_num, oss);
    if(remaining_fragments.size() > 0)
        THROW_ERROR("Did not use all fragments");
    // Make the actual rule
    // TODO: If we have target labels, add them
    oss << " |||";
    int last = -1;
    for(int i = 0; i < (int)trg_cover.size(); i++) {
        if(trg_cover[i] == -1)
            oss << " \"" << Dict::WSym(trg_sent[i+trg_begin]) << "\"";
        else if (last != trg_cover[i]) {
            oss << " x" << trg_cover[i];
            last = trg_cover[i];
        }
    }
    if(tail_labs.size() != 0) {
        oss << " @";
        BOOST_FOREACH(WordId wid, tail_labs)
            oss << ' ' << (wid == -1 ? Dict::INVALID_SPAN_SYMBOL : Dict::WSym(wid));
    }
    oss << " ||| " << exp(rule.GetScore());
    return oss.str();
}

string PrintPhrasePairH(PhrasePair pp) {
    ostringstream oss;
    oss << "[(" << pp.first.first << "," << pp.first.second << ")(" << pp.second.first << "," << pp.second.second << ")]";
    return oss.str();
} 

//////////////////////////////////////////
//        HIERO RULE EXTRACTOR          //
//////////////////////////////////////////

// Public 
std::vector<vector<HieroRule> > HieroExtractor::ExtractHieroRule(const Alignment & align, const Sentence & source, 
        const Sentence & target) 
{
    vector<vector<HieroRule> >ret = vector<vector<HieroRule> >();
    PhrasePairs filtered_pairs = PhrasePairs();
    PhrasePairs pairs;

    // Doing extraction safely
    try {
        pairs = ExtractPhrase(align,source, target);
    } catch (std::exception& exc) {
        THROW_ERROR("Input or alignment error. \n\tOn Source: " + Dict::PrintWords(source) + 
            "\n\tOn Target: " + Dict::PrintWords(target));
    }

    int rule_max_len = HieroExtractor::GetMaxRuleLen();

    // if there are multiple initial phrase pairs containing the same set of alignments, only the 
    // smallest is kept. That is, unaligned words are not allowed at the edges of phrases
    map<pair<int,int>, int> mp = map<pair<int,int>,int>();
    BOOST_FOREACH(PhrasePair pp, pairs) {
        pair<int,int> src = pp.first;
        pair<int,int> trg = pp.second;
        pair<int,int> index = make_pair(src.first,src.second);
        int len = trg.second-trg.first;

        if (mp.find(index)==mp.end() || mp[index] > len) {
            mp[index] = len;
        }
    }
    
    BOOST_FOREACH(PhrasePair pp, pairs) {
        pair<int,int> src = pp.first;
        pair<int,int> trg = pp.second;
        pair<int,int> index = make_pair(src.first,src.second);
        int len = trg.second-trg.first;
        if (len == mp[index]) {
            filtered_pairs.push_back(pp);
        }
    }
    pairs = filtered_pairs;
    // Rule extraction algorithm for 1 + 2 NonTerminal Symbol. If we use Higher, algorithm is too complex
    for (int ii=0; (unsigned) ii < pairs.size(); ++ii) {
        // initial phrases are limited to a length of INITIAL_PHRASE_LIMIT (10) words on either side
        if (pairs[ii].first.second - pairs[ii].first.first < HieroExtractor::GetMaxInitialPhrase() || 
                pairs[ii].second.second - pairs[ii].second.first < HieroExtractor::GetMaxInitialPhrase()) 
        {
            vector<HieroRule> _extracted = vector<HieroRule>();
            _extracted.push_back(ParsePhraseTranslationRule(source,target,pairs[ii]));
            // Find pairs of 2 rules
            for (int jj=0; (unsigned) jj < pairs.size(); ++jj) {
                if (jj != ii && InPhrase(pairs[jj],pairs[ii])) {
                    for (int kk=jj+1; (unsigned) kk < pairs.size(); ++kk) {
                        // that are in the span of INITIAL phrase, and NOT overlapping each other
                        if (kk != jj && InPhrase(pairs[kk],pairs[ii]) && InPhrase(pairs[jj],pairs[ii]) && !IsPhraseOverlapping(pairs[jj],pairs[kk])) 
                        {
                            HieroRule _rule = ParseBinaryPhraseRule(source,target,pairs[jj],pairs[kk],pairs[ii]);
                            if ((int)_rule.GetSourceSentence().size() <= rule_max_len || 
                                    (int) _rule.GetTargetSentence().size() <= rule_max_len)
                            {
                                HieroRuleManager::AddRule(_extracted,_rule);
                            }
                        }
                    }
                    // Unary rule
                    HieroRule _rule = ParseUnaryPhraseRule(source,target,pairs[jj],pairs[ii]);
                    if ((int)_rule.GetSourceSentence().size() <= rule_max_len || 
                            (int) _rule.GetTargetSentence().size() <= rule_max_len)
                    {
                        HieroRuleManager::AddRule(_extracted,_rule);
                    }
                }
            }
            if (_extracted.size() > (unsigned) 0) {
                ret.push_back(_extracted);
            }
        }
        // 
    }
    // DEBUG PrintPhrasePairs();
    return ret;
}

// The implementation of phrase extraction algorithm.
// The algorithm to extract all consistent phrase pairs from a word-aligned sentence pair
PhrasePairs HieroExtractor::ExtractPhrase(const Alignment & align, const Sentence & source, 
        const Sentence & target) 
{
    std::vector<std::set<int> > s2t = align.GetSrcAlignments();
    std::vector<std::set<int> > t2s = std::vector<std::set<int> >();
    PhrasePairs ret = PhrasePairs();

    // Remap from source -> target back to target -> source
    for (unsigned t=0; t < target.size(); ++t) {
        t2s.push_back(set<int>());
    }
    for (unsigned s=0; s < s2t.size(); ++s) {
        set<int> ts = s2t[s];
        BOOST_FOREACH(int t, ts) {
            t2s[t].insert((int)s);
        }
    }

    // Phrase Extraction Algorithm 
    // This is very slow (actually), maybe better data structure can improves it.
    for (int s_begin=0; s_begin < (int)s2t.size(); ++s_begin) {
        std::map<int, int> tp;
        for (int s_end=s_begin; s_end < (int)s2t.size(); ++s_end) {
            if (s2t[s_end].size() != 0) { 
                BOOST_FOREACH(int _t, s2t[s_end]) { tp[_t]++;}
            }
            int t_begin = MapMinKey(tp);
            int t_end = MapMaxKey(tp);
            if (QuasiConsecutive(t_begin,t_end,tp,t2s)) {
                std::map<int, int> sp;
                for (int t=t_begin; t<=t_end;++t) {
                    if (t2s[t].size() != 0) {
                        BOOST_FOREACH(int _s, t2s[t]) { sp[_s]++; }
                    }
                }
                if (MapMinKey(sp) >= s_begin && MapMaxKey(sp) <= s_end) {
                    while (t_begin >= 0) {
                        int jp = t_end;
                        while (jp <= (int)t2s.size()) {
                            ret.push_back(make_pair(make_pair(s_begin,s_end),make_pair(t_begin,jp)));
                            ++jp;   
                            if (jp == (int)t2s.size() || t2s[jp].size() != 0) {
                                break;
                            }
                        }
                        --t_begin;
                        if(t_begin < 0 || t2s[t_begin].size() != 0) {
                            break;
                        }
                        
                    }
                }
            }
        }
    }
    return ret;
}

// Private Member
string HieroExtractor::AppendString(const Sentence & s, int begin, int end) {
    string ret = string("");
    for (int i=begin; i<=(int)end; ++i) {
        ret += Dict::WSym(s[i]);
        if (i != end) {
            ret += " ";
        }
    }
    return ret;
}

string HieroExtractor::PrintPhrasePair(const PhrasePair & pp, const Sentence & source, 
    const Sentence & target) 
{
    return AppendString(source, pp.first.first, pp.first.second) + string(" -> ") 
        + AppendString(target, pp.second.first, pp.second.second);
}

void HieroExtractor::PrintPhrasePairs(const PhrasePairs & pairs, const Sentence & source, 
    const Sentence & target) 
{
    BOOST_FOREACH(PhrasePair pp , pairs) {
        cerr << PrintPhrasePair(pp,source,target) <<endl;
    }
}

int HieroExtractor::MapMaxKey(const std::map<int,int> & map) {
    int max = 0;
    pair<int,int> item;
    BOOST_FOREACH(item, map) {
        if (item.first > max) max = item.first;
    } 
    return max;
}

int HieroExtractor::MapMinKey(const std::map<int,int> & map) {
    int min = 0;
    int is_first = 1;
    pair<int,int> item;
    BOOST_FOREACH(item, map) {
        if (is_first || item.first < min) {
            is_first = 0;
            min = item.first;
        }
    } 
    return min;
}

int HieroExtractor::QuasiConsecutive(int small, int large, const map<int,int> & tp, 
        const vector<set<int> > & t2s) 
{
    for (int i=small; i <= large; ++i) {
        if (t2s[i].size() != 0 && tp.find(i) == tp.end()) {
            return 0;
        }
    }   
    return 1;
}

int HieroExtractor::IsTerritoryOverlapping(const pair<int,int> & a, const pair<int,int> & b) {
    return 
        (a.first >= b.first && a.second <= b.second) || // a in b
        (b.first >= a.first && b.second <= a.second) || // b in a
        (a.first <= b.first && a.second >= b.first)  || // a preceeds b AND they are overlapping
        (b.first <= a.first && b.second >= a.first);    // b preceeds a AND they are overlapping
}

int HieroExtractor::IsPhraseOverlapping(const PhrasePair & pair1, const PhrasePair & pair2) {
    return IsTerritoryOverlapping(pair1.first, pair2.first) || IsTerritoryOverlapping(pair1.second,pair2.second);
}

void HieroExtractor::ParseRuleWith2NonTerminals(const Sentence & sentence, const std::pair<int,int> & pair1, 
        const std::pair<int,int> & pair2, 
        const std::pair<int,int> & pair_span, 
        HieroRule & target, int type) 
{
    target.SetType(type);
    int x0 = 0;
    int x1 = 0;
    for (int i=pair_span.first; i <= pair_span.second; ++i) {
        if (i >= pair1.first && i <= pair1.second) {
            if (x1) {
                target.AddNonTermX(1);
                x1 = 0;
            }
            x0 = 1;
        } else if (i >= pair2.first && i <= pair2.second) {
            if (x0) {
                target.AddNonTermX(0);
                x0 = 0;
            }
            x1 = 1;
        } else {
            if (x0) {
                target.AddNonTermX(0);
                x0 = 0;
            } 
            if (x1) {
                target.AddNonTermX(1);
                x1 = 0;
            }
            target.AddWord(sentence[i]);
        }
    }
    if (x0) target.AddNonTermX(0);
    else if (x1) target.AddNonTermX(1);
}

HieroRule HieroExtractor::ParseBinaryPhraseRule(const Sentence & source, const Sentence & target, const PhrasePair & pair1, 
        const PhrasePair & pair2, const PhrasePair & pair_span) 
{
    HieroRule _rule = HieroRule();
    ParseRuleWith2NonTerminals(source,pair1.first,pair2.first,pair_span.first,_rule,HIERO_SOURCE);
    ParseRuleWith2NonTerminals(target,pair1.second,pair2.second,pair_span.second,_rule,HIERO_TARGET);
    return _rule;
}

void HieroExtractor::ParseRuleWith1NonTerminals(const Sentence & sentence, const std::pair<int,int> & pair, 
        const std::pair<int,int> & pair_span, HieroRule & target, int type) 
{
    target.SetType(type);
    int x = 0;
    for (int i=pair_span.first; i <= pair_span.second; ++i) {
        if (i >= pair.first && i <= pair.second) {
            x = 1;
        } else {
            if (x) {
                target.AddNonTermX(0);
                x = 0; 
            } 
            target.AddWord(sentence[i]);
        }
    }
    if (x) target.AddNonTermX(0);
}

HieroRule HieroExtractor::ParseUnaryPhraseRule(const Sentence & source, const Sentence & target, 
        const PhrasePair & pair, const PhrasePair & pair_span) 
{
    HieroRule _rule = HieroRule();
    ParseRuleWith1NonTerminals(source,pair.first,pair_span.first,_rule,HIERO_SOURCE);
    ParseRuleWith1NonTerminals(target,pair.second,pair_span.second,_rule,HIERO_TARGET);
    return _rule;
}

HieroRule HieroExtractor::ParsePhraseTranslationRule(const Sentence & source, const Sentence & target, 
        const PhrasePair & pair) 
{
    HieroRule _rule = HieroRule();
    _rule.SetType(HIERO_SOURCE);
    for (int i=pair.first.first; i <= pair.first.second; ++i) _rule.AddWord(source[i]);
    _rule.SetType(HIERO_TARGET);
    for (int i=pair.second.first; i <= pair.second.second; ++i) _rule.AddWord(target[i]);
    return _rule;
}

int HieroExtractor::InPhrase(const PhrasePair & p1, const PhrasePair & p2) {
    return p1.first.first >= p2.first.first && p1.first.second <= p2.first.second && 
            p1.second.first >= p2.second.first && p1.second.second <= p2.second.second;
}

