#include <boost/tuple/tuple.hpp>
#include <travatar/rule-extractor.h>
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
            old_new_ids.insert(MakePair(v->GetId(), ret->NumNodes()));
            HyperNode* node = new HyperNode(v->GetSym(), v->GetSpan(), ret->NumNodes());
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
                    BOOST_FOREACH(HyperNode* t, e->GetTails()) {
                        // If this is not a frontier node, push it on the queue
                        if(t->IsFrontier() != HyperNode::IS_FRONTIER)
                            new_front.push_back(t);
                        else
                            frag_front.first->AddTail(ret->GetNode(old_new_ids[t->GetId()]));
                    }
                    BOOST_FOREACH(HyperNode * stack_node, frag_front.second)
                        new_front.push_back(stack_node);
                    // Push this on to the queue
                    HyperEdge * new_frag(new HyperEdge(*frag_front.first));
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
    vector<bool> nulls(trg_len, true);
    BOOST_FOREACH(const Alignment::AlignmentPair & a, align.GetAlignmentVector())
        nulls[a.second] = false;
    AttachNullsTop(nulls, *ret->GetNode(0));
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
    ret->GetNodes().resize(0);
    ret->GetEdges().resize(0);
    ret->AddNode(new HyperNode);
    vector<bool> nulls(trg_len, true);
    // Find the null aligned target words
    BOOST_FOREACH(const Alignment::AlignmentPair &a, align.GetAlignmentVector())
        nulls[a.second] = false;
    vector< SpanNodeVector > new_nodes(rule_graph.NumNodes());
    // Get the expanded nodes in the new graph 
    GetExpandedNodes(nulls, *rule_graph.GetNode(0), new_nodes);
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
                            vector<ForestExtractor::SpanNodeVector> & expanded
                ) {
    int old_id = old_node.GetId();
    if(expanded[old_id].size())
        return expanded[old_id];
    // Expand the node itself
    expanded[old_id] = ExpandNode(nulls, old_node);
    // Create the stack of partially processed tails
    typedef boost::tuple<set<int>, const HyperEdge*, HyperEdge*> q_tuple;
    queue< q_tuple > open_queue;
    // If the node has tails, expand the tails for each node
    BOOST_FOREACH(const SpanNodeVector::value_type & val, expanded[old_id]) {
        BOOST_FOREACH(const HyperEdge* edge, old_node.GetEdges()) {
            HyperEdge * new_edge = new HyperEdge(*edge);
            new_edge->SetHead(val.second);
            new_edge->SetId(-1);
            new_edge->GetTails().resize(0);
            open_queue.push(q_tuple(val.first, edge, new_edge));
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
            const SpanNodeVector & next_exp = GetExpandedNodes(nulls, 
                                                               *trip.get<1>()->GetTail(trip.get<2>()->NumTails()),
                                                               expanded);
            // For all the possibilities
            BOOST_FOREACH(const SpanNodeVector::value_type & val, next_exp) {
                set<int> new_set = trip.get<0>();
                bool ok = true;
                // For each value covered by the tail, check to make sure that
                // we do not have any doubly covered nulls
                BOOST_FOREACH(int covered, val.first) {
                    ok = (new_set.find(covered) == new_set.end());
                    if(!ok) break;
                    new_set.insert(covered);
                }
                // If we are OK, create a new edge and add it
                if(ok) {
                    HyperEdge * next_edge = new HyperEdge(*trip.get<2>());
                    next_edge->SetId(-1);
                    next_edge->AddTail(val.second);
                    open_queue.push(q_tuple(new_set, trip.get<1>(), next_edge));
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
                const HyperNode & old_node) const {
    SpanNodeVector ret;
    const set<int> & trg_span = old_node.GetTrgSpan();
    // For nodes that are entirely unaligned
    if(trg_span.size() == 0) {
        HyperNode * next_node = new HyperNode(old_node);
        next_node->SetId(-1);
        ret.push_back(MakePair(set<int>(), next_node));
        return ret;
    }
    // i_node stores the new node with all nulls before
    set<int> i_set;
    for(int i = *trg_span.begin(); i >= 0 && (i == *trg_span.begin() || nulls[i]); i--) {
        if(nulls[i]) i_set.insert(i);
        set<int> j_set = i_set;
        for(int j = *trg_span.rbegin();
              j < (int)nulls.size() && (j == *trg_span.rbegin() || nulls[j]); 
              j++) {
            if(nulls[j]) j_set.insert(j);
            HyperNode * j_node = new HyperNode(old_node);
            j_node->GetEdges().resize(0);
            j_node->SetId(-1);
            BOOST_FOREACH(int k, j_set)
                j_node->GetTrgSpan().insert(k);
            ret.push_back(MakePair(j_set, j_node));
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

// Creating a rule
string RuleExtractor::RuleToString(const HyperEdge & rule, const Sentence & src_sent, const Sentence & trg_sent) const {
    // Get the target span for the top node
    const std::set<int> & trg_span = rule.GetHead()->GetTrgSpan();
    int trg_begin = *trg_span.begin(), trg_end = *trg_span.rbegin()+1;
    // Create the covered target vector. Initially all are -1, indicating that
    // they have not yet been covered by a child frontier node
    vector<int> trg_cover(trg_end-trg_begin, -1);
    const vector<HyperNode*> & tails = rule.GetTails();
    map<int,int> tail_map;
    for(int i = 0; i < (int)tails.size(); i++) {
        tail_map[tails[i]->GetId()] = i;
        const std::set<int> & my_trg_span = tails[i]->GetTrgSpan();
        for(int j = *my_trg_span.begin(); j <= *my_trg_span.rbegin(); j++)
            trg_cover[j-trg_begin] = i;
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
    oss << " ||| " << exp(rule.GetScore());
    return oss.str();
}
