
#include <travatar/lookup-table.h>

using namespace travatar;
using namespace boost;
using namespace std;

// Find all the translation rules rooted at a particular node in a parse graph
vector<shared_ptr<LookupState> > LookupTable::LookupSrc(
            const HyperNode & node, 
            const vector<shared_ptr<LookupState> > & old_states) {
    vector<shared_ptr<LookupState> > ret_states;
    BOOST_FOREACH(const shared_ptr<LookupState> & state, old_states) {
        // Match the current node
        shared_ptr<LookupState> my_state(MatchNode(node, *state));
        if(my_state != NULL) ret_states.push_back(my_state);
        // Match all rules the require descent into the next node
        shared_ptr<LookupState> start_state(MatchStart(node, *state));
        if(start_state == NULL) continue;
        // Cycle through all the edges from this node
        BOOST_FOREACH(const HyperEdge * edge, node.GetEdges()) {
            vector<shared_ptr<LookupState> > my_states(1, start_state);
            // Cycle through all the tails in this edge
            BOOST_FOREACH(const HyperNode * child, edge->GetTails())
                my_states = LookupSrc(*child, my_states);
            // Finish all the states found
            BOOST_FOREACH(const shared_ptr<LookupState> & my_state, my_states) {
                shared_ptr<LookupState> fin_state(MatchEnd(node, *my_state));
                if(fin_state != NULL) ret_states.push_back(fin_state);
            }
        }
        
    }
    return ret_states;
}
