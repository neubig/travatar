#include <travatar/rule-extractor.h>
#include <queue>
#include <stack>

using namespace travatar;
using namespace std;
using namespace boost;

const set<int> * ForestExtractor::CalculateSpan(
                   const HyperNode * node,
                   const vector<set<int> > & src_spans,
                   vector<set<int>*> & trg_spans) const {
    // Memoized recursion
    int id = node->GetId();
    if(trg_spans[id] != NULL) return trg_spans[id];
    // If this is terminal, simply set to aligned values
    if(node->IsTerminal()) {
        trg_spans[id] = new set<int>(src_spans[node->GetSpan().first]);
    } else {
        // First, calculate all the spans
        trg_spans[id] = new set<int>;
        BOOST_FOREACH(HyperNode* child, node->GetEdge(0)->GetTails()) {
            BOOST_FOREACH(int val, *CalculateSpan(child,src_spans,trg_spans)) {
                trg_spans[id]->insert(val);
            }
        }
    }
    return trg_spans[id];
}

// Calculate whether each node is on the frontier or not.
// At the moment, we will treat terminals as non-frontier nodes, and only
// extract words that are rooted at a non-terminal.
int ForestExtractor::CalculateFrontier(
                   HyperNode * node,
                   const vector<set<int> > & src_spans,
                   vector<set<int>*> & trg_spans,
                   const set<int> & complement) const {
    // Check if this is in the frontier
    HyperNode::FrontierType ret =
        (node->IsTerminal() ? HyperNode::NOT_FRONTIER : HyperNode::IS_FRONTIER);
    const set<int>* span = CalculateSpan(node, src_spans, trg_spans);
    for(set<int>::const_iterator it = span->begin();
        ret == HyperNode::IS_FRONTIER && it != span->end();
        it++)
        if(complement.find(*it) != complement.end())
            ret = HyperNode::NOT_FRONTIER;
    node->SetFrontier(ret);
    // For all other nodes
    BOOST_FOREACH(HyperEdge * edge, node->GetEdges()) {
        vector<HyperNode*> & tails = edge->GetTails();
        BOOST_FOREACH(HyperNode* child, tails) {
            if(child->IsFrontier() != HyperNode::UNSET_FRONTIER) continue;
            set<int> my_comp = complement;
            BOOST_FOREACH(HyperNode* child2, tails) {
                if(child != child2) {
                    BOOST_FOREACH(int pos, *trg_spans[child2->GetId()])
                        my_comp.insert(pos);
                }
            }
            CalculateFrontier(child, src_spans, trg_spans, my_comp);
        }
    }
    return ret;
}

// Create a rule graph using Mi and Huang's forest-based rule extraction algorithm
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
vector<shared_ptr<GraphFragment> > ForestExtractor::ExtractRules(
        HyperGraph & src_parse,
        const Sentence & trg_sent,
        const Alignment & align) const {
    // Calculate spans and the frontier set (GHKM)
    vector<set<int> > src_spans = align.GetSrcAlignments();
    vector<set<int>*> trg_spans(src_parse.NumNodes(), (set<int>*)NULL);
    CalculateFrontier(src_parse.GetNode(0), src_spans, trg_spans, set<int>());
    // Contains a rule and the as-of-yet-unexpanded nodes
    typedef pair<shared_ptr<GraphFragment>,deque<HyperNode*> > FragFront;
    // Create the rules
    // See Mi and Huang Algorithm 1 for pseudo-code
    vector<shared_ptr<GraphFragment> > ret;
    BOOST_FOREACH(HyperNode * v, src_parse.GetNodes()) {
        // Skip non-frontier nodes
        if(v->IsFrontier() != HyperNode::IS_FRONTIER) continue;
        // Create the queue of nodes to process
        stack<FragFront> open;
        shared_ptr<GraphFragment> frag(new GraphFragment);
        deque<HyperNode*> front; front.push_back(v);
        open.push(FragFront(frag, front));
        // Continue processing until the queue is empty
        while(open.size() != 0) {
            FragFront frag_front = open.top(); open.pop();
            // If there are no more nodes to expand, add the rule
            if(frag_front.second.size() == 0)
                ret.push_back(frag_front.first);
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
                // We want to push these on the stack backwards so we can process
                // in ascending order
                BOOST_REVERSE_FOREACH(HyperEdge* e, u->GetEdges()) {
                    // Calculate the nodes that still need to be expanded
                    deque<HyperNode*> new_front = frag_front.second;
                    BOOST_FOREACH(HyperNode* t, e->GetTails()) {
                        if(t->IsFrontier() != HyperNode::IS_FRONTIER)
                            new_front.push_back(t);
                    }
                    // Push this on to the queue
                    shared_ptr<GraphFragment> new_frag(new GraphFragment(*frag_front.first));
                    new_frag->AddEdge(e);
                    open.push(FragFront(new_frag, new_front));
                }
            }
        }

    }
    return ret;
}
