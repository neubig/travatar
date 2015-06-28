#include <queue>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/tr1/unordered_set.hpp>
#include <lm/left.hh>
#include <lm/model.hh>
#include <travatar/weights.h>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>
#include <travatar/io-util.h>
#include <travatar/check-equal.h>
#include <travatar/softmax.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;
using namespace boost;
using namespace lm::ngram;


void HyperEdge::AddTail(HyperNode* tail) {
    if(head_ && head_->GetId() != -1 && head_->GetId() == tail->GetId())
        THROW_ERROR("Adding head and tail to the same edge: " << head_->GetId());
    tails_.push_back(tail);
}

// Refresh the pointers to head and tail nodes so they point to
// nodes in a new HyperGraph. Useful when copying nodes
void HyperEdge::RefreshPointers(HyperGraph & new_graph) {
    head_ = new_graph.GetNode(head_->GetId());
    for(int i = 0; i < (int)tails_.size(); i++)
        tails_[i] = new_graph.GetNode(tails_[i]->GetId());
}


// Refresh the pointers to head and tail nodes so they point to
// nodes in a new HyperGraph. Useful when copying nodes
void HyperNode::RefreshPointers(HyperGraph & new_graph) {
    for(int i = 0; i < (int)edges_.size(); i++)
        edges_[i] = new_graph.GetEdge(edges_[i]->GetId());
}


// Check to make sure that two hyperedges are equal
bool HyperEdge::operator==(const HyperEdge & rhs) const {
    if(id_ != rhs.id_ ||
       (head_==NULL) != (rhs.head_==NULL) ||
       (head_!=NULL && head_->GetId() != rhs.head_->GetId()) ||
       (tails_.size() != rhs.tails_.size()))
        return false;
    for(int i = 0; i < (int)tails_.size(); i++)
       if((tails_[i]==NULL) != (rhs.tails_[i]==NULL) ||
          (tails_[i]!=NULL && tails_[i]->GetId() != rhs.tails_[i]->GetId()))
          return false;
    if(features_ != rhs.features_)
        return false;
    if(src_str_ != rhs.src_str_)
        return false;
    for(int i = 0; i < (int)trg_data_.size(); i++)
        if(trg_data_ != rhs.trg_data_)
            return false;
    if(abs(score_ - rhs.score_) > abs(score_*1e-4))
        return false;
    return true;
}

// This is a Bad copy and paste
// TODO: fix it
bool RuleEdge::operator==(const RuleEdge & rhs) const {
    if(id_ != rhs.id_ ||
       (head_==NULL) != (rhs.head_==NULL) ||
       (head_!=NULL && head_->GetId() != rhs.head_->GetId()) ||
       (fragment_edges_.size() != rhs.fragment_edges_.size()) ||
       (tails_.size() != rhs.tails_.size()))
        return false;
    for(int i = 0; i < (int)tails_.size(); i++)
       if((tails_[i]==NULL) != (rhs.tails_[i]==NULL) ||
          (tails_[i]!=NULL && tails_[i]->GetId() != rhs.tails_[i]->GetId()))
          return false;
    for(int i = 0; i < (int)fragment_edges_.size(); i++)
       if((fragment_edges_[i]==NULL) != (rhs.fragment_edges_[i]==NULL) ||
          (fragment_edges_[i]!=NULL && fragment_edges_[i]->GetId() != rhs.fragment_edges_[i]->GetId()))
          return false;
    if(features_ != rhs.features_)
        return false;
    if(src_str_ != rhs.src_str_)
        return false;
    for(int i = 0; i < (int)trg_data_.size(); i++)
        if(trg_data_ != rhs.trg_data_)
            return false;
    if(abs(score_ - rhs.score_) > abs(score_*1e-4))
        return false;
    return true;
}

// Output for a hyperedge in JSON format
void HyperEdge::Print(std::ostream & out) const {
    out << "{\"id\": "<<id_<<", \"head\": "<<(head_ == NULL ? -1 : head_->GetId());
    if(tails_.size()) {
        out << ", \"tails\": [";
        for(int i = 0; i < (int)tails_.size(); i++)
            out << tails_[i]->GetId() << ((i == (int)tails_.size()-1) ? "]" : ", ");
    }
    if(trg_data_.size()) {
        out << ", \"trg\": [";
        for(int j = 0; j < (int)trg_data_.size(); j++)
            out << trg_data_[j] << (j == (int)trg_data_.size()-1 ? "]" : ", ");
    }
    if(features_.size())
        out << ", \"features\": " << features_;
    if(src_str_.size())
        out << ", \"src_str\": \"" << EscapeQuotes(src_str_) << '"';
    // if(fragment_edges_.size()) {
    //     out << ", \"fragment_edges\": [";
    //     for(int i = 0; i < (int)fragment_edges_.size(); i++)
    //         out << fragment_edges_[i]->GetId() << ((i == (int)fragment_edges_.size()-1) ? "]" : ", ");
    // }
    if(score_ != 0)
        out << ", \"score\": " << score_;
    out << "}";
}

// Check to make sure that two hypernodes are equal
bool HyperNode::operator==(const HyperNode & rhs) const {
    if(id_ != rhs.id_ || src_span_ != rhs.src_span_ ||
       sym_ != rhs.sym_ || edges_.size() != rhs.edges_.size())
        return false;
    for(int i = 0; i < (int)edges_.size(); i++)
        if((edges_[i]==NULL) != (rhs.edges_[i]==NULL) ||
            (edges_[i]!=NULL && edges_[i]->GetId() != rhs.edges_[i]->GetId()))
            return false;
    if(trg_span_ != rhs.trg_span_)
        return false;
    if(frontier_ != rhs.frontier_)
        return false;
    if(abs(viterbi_score_ - rhs.viterbi_score_) > 1e-6)
        return false;
    return true;
}

// Output for a hypernode in JSON format
void HyperNode::Print(std::ostream & out) const {
    out << "{\"id\": "<<id_;
    if(sym_ != -1)
        out << ", \"sym\": " << Dict::WSymEscaped(sym_);
    out << ", \"span\": "<<src_span_;
    // if(edges_.size()) {
    //     out << ", \"edges\": [";
    //     for(int i = 0; i < (int)edges_.size(); i++)
    //         out << edges_[i]->GetId() << ((i == (int)edges_.size()-1) ? "]" : ", ");
    // }
    if(trg_span_.size() > 0) {
        out << ", \"trg_span\": [";
        int num = 0;
        BOOST_FOREACH(int v, trg_span_)
            out << (num++ != 0?", ":"") << v;
        out << "]";
    }
    if(viterbi_score_ != -REAL_MAX)
        out << ", \"viterbi\": " << viterbi_score_;
    if(frontier_ != UNSET_FRONTIER)
        out << ", \"frontier\": \"" << (char)frontier_<<"\"";
    out << "}";
}


// Check to make sure two hypergraphs are equal
int HyperGraph::CheckEqual(const HyperGraph & rhs) const {
    return CheckPtrVector(edges_, rhs.edges_) &&
           CheckPtrVector(nodes_, rhs.nodes_) &&
           CheckVector(words_, rhs.words_);
}

// Check to make sure two hypergraphs are equal
int HyperGraph::CheckMaybeEqual(const HyperGraph & rhs) const {
    // Check if nodes and edges are equal
    if(!(edges_.size() == rhs.edges_.size() &&
         nodes_.size() == rhs.nodes_.size() &&
         CheckVector(words_, rhs.words_))) {
        cerr << "edges: " << edges_.size() << " == " << rhs.edges_.size() << endl;
        BOOST_FOREACH(HyperEdge* edge, this->GetEdges()) { edge->Print(cerr); cerr << endl; }
        BOOST_FOREACH(HyperEdge* edge, rhs.GetEdges())   { edge->Print(cerr); cerr << endl; }
        cerr << endl << "nodes: " << nodes_.size() << " == " << rhs.nodes_.size() << endl;
        BOOST_FOREACH(HyperNode* node, this->GetNodes()) { node->Print(cerr); cerr << endl; }
        BOOST_FOREACH(HyperNode* node, rhs.GetNodes())   { node->Print(cerr); cerr << endl; }
        cerr << endl;
        return 0;
    }
    // Check if tails are equal
    vector<int> exp_tails, act_tails;
    BOOST_FOREACH(const HyperEdge * edge, edges_) exp_tails.push_back(edge->GetTails().size());
    BOOST_FOREACH(const HyperEdge * edge, rhs.edges_) act_tails.push_back(edge->GetTails().size());
    sort(exp_tails.begin(), exp_tails.end());
    sort(act_tails.begin(), act_tails.end());
    if(!CheckVector(exp_tails, act_tails)) return 0;
    // Check if scores are equal
    vector<Real> l_scores(nodes_.size()), r_scores(nodes_.size());
    for(int i = 0; i < (int)nodes_.size(); i++) {
        l_scores[i] = nodes_[i]->GetViterbiScore();
        r_scores[i] = rhs.nodes_[i]->GetViterbiScore();
    }
    sort(l_scores.begin(), l_scores.end());
    sort(r_scores.begin(), r_scores.end());
    return CheckAlmostVector(l_scores, r_scores);
}

const set<int> & HyperNode::CalculateTrgSpan(
        const vector<set<int> > & word_spans) {
    // Memoized recursion
    if(has_trg_span_) return trg_span_;
    has_trg_span_ = true;
    // If this is terminal, simply set to aligned values
    if(IsTerminal()) {
        // Skip null values
        if(src_span_.first < (int)word_spans.size())
            trg_span_ = word_spans[src_span_.first];
    } else {
        // First, calculate all the spans
        trg_span_ = set<int>();
        BOOST_FOREACH(HyperNode* child, GetEdge(0)->GetTails()) {
            BOOST_FOREACH(int val, child->CalculateTrgSpan(word_spans)) {
                trg_span_.insert(val);
            }
        }
    }
    return trg_span_;
}

// Calculate whether each node is on the frontier or not.
// At the moment, we will treat terminals as non-frontier nodes, and only
// extract words that are rooted at a non-terminal.
HyperNode::FrontierType HyperNode::CalculateFrontier(
                   const vector<set<int> > & src_spans,
                   const set<int> & complement) {
    if(frontier_ != UNSET_FRONTIER) return frontier_;
    if(IsTerminal()) return (frontier_ = HyperNode::NOT_FRONTIER);
    // Check if this is in the frontier
    CalculateTrgSpan(src_spans);
    // We define null-aligned words to not be on the frontier
    if(trg_span_.size() != 0) {
        frontier_ = HyperNode::IS_FRONTIER;
        for(int i = *trg_span_.begin(); frontier_ == HyperNode::IS_FRONTIER && i <= *trg_span_.rbegin(); i++)
            if(complement.find(i) != complement.end())
                frontier_ = HyperNode::NOT_FRONTIER;
    } else {
        frontier_ = HyperNode::NOT_FRONTIER;
    }
    // For all other nodes
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        vector<HyperNode*> & tails = edge->GetTails();
        BOOST_FOREACH(HyperNode* child, tails) {
            if(child->IsFrontier() != HyperNode::UNSET_FRONTIER) continue;
            set<int> my_comp = complement;
            BOOST_FOREACH(HyperNode* child2, tails) {
                if(child != child2) {
                    BOOST_FOREACH(int pos, child2->CalculateTrgSpan(src_spans))
                        my_comp.insert(pos);
                }
            }
            child->CalculateFrontier(src_spans, my_comp);
        }
    }
    return frontier_;
}

void HyperPath::AddEdges(std::vector<HyperEdge*> & edges) {
    BOOST_FOREACH(HyperEdge* edge, edges)
        edges_.push_back(edge);
}

SparseVector HyperPath::GetFeatures() {
    SparseVector ret;
    BOOST_FOREACH(const HyperEdge* edge, edges_)
        ret += edge->GetFeatures();
    return ret;
}

class RankScoreMore {
public:
    bool operator()(const std::pair<Real, int> & x, const std::pair<Real, int> & y) {
        if(x.first != y.first) { return x.first > y.first; }
        return x.second < y.second;
    }
};

boost::shared_ptr<NbestState> HyperGraph::NbestCalcState(int node, vector<boost::shared_ptr<NbestState> > & states) {
    if(states[node].get() == NULL) {
        states[node].reset(new NbestState);
        // Get the edges and sort in descending order of score
        BOOST_FOREACH(HyperEdge* edge, nodes_[node]->GetEdges()) {
            Real score = edge->GetScore();
            BOOST_FOREACH(HyperNode* node, edge->GetTails())
                score += node->CalcViterbiScore();
            states[node]->edges.push_back(make_pair(score, edge->GetId()));
        }
        sort(states[node]->edges.begin(), states[node]->edges.end(), RankScoreMore());
        // Initialize stack with all edges
        for(int i = 0; i < (int)states[node]->edges.size(); i++) {
            NbestEdge n_edge = states[node]->edges[i];
            states[node]->PushStack(boost::shared_ptr<NbestStackItem>(new NbestStackItem(n_edge.first, i, vector<int>(edges_[n_edge.second]->GetTails().size(), 0))));
        }
    }
    return states[node];
}

boost::shared_ptr<HyperPath> HyperGraph::NbestCreatePath(int node, NbestStackItem & item, bool uniq, vector<boost::shared_ptr<NbestState> > & states) {
    boost::shared_ptr<HyperPath> ret(new HyperPath);
    ret->SetScore(item.score);
    HyperEdge* my_edge = edges_[states[node]->edges[item.edge_rank].second];
    ret->AddEdge(my_edge);
    for(int i = 0; i < (int)item.child_ranks.size(); i++) {
        boost::shared_ptr<HyperPath> child_path = NbestGetNthPath(my_edge->GetTail(i)->GetId(), item.child_ranks[i], uniq, states);
        if(child_path.get() == NULL) return child_path;
        ret->AddEdges(child_path->GetEdges());
    }
    return ret;
}

boost::shared_ptr<NbestStackItem> HyperGraph::NbestStackIncrementChild(int node, NbestStackItem & item, int child_num, bool uniq, vector<boost::shared_ptr<NbestState> > & states) {
    HyperEdge* my_edge = edges_[states[node]->edges[item.edge_rank].second];
    boost::shared_ptr<HyperPath> curr_path = NbestGetNthPath(my_edge->GetTail(child_num)->GetId(), item.child_ranks[child_num], uniq, states);
    boost::shared_ptr<HyperPath> next_path = NbestGetNthPath(my_edge->GetTail(child_num)->GetId(), item.child_ranks[child_num]+1, uniq, states);
    if(next_path.get() == NULL)
        return boost::shared_ptr<NbestStackItem>();
    Real diff = next_path->GetScore() - curr_path->GetScore();
    boost::shared_ptr<NbestStackItem> ret(new NbestStackItem(item));
    ret->score += diff;
    ret->child_ranks[child_num]++;
    return ret;
}

boost::shared_ptr<HyperPath> HyperGraph::NbestGetNthPath(int node, int rank, bool uniq, vector<boost::shared_ptr<NbestState> > & states) {
    boost::shared_ptr<NbestState> state = NbestCalcState(node, states);
    // Until we've found a path or the stack is exhausted
    while(state->paths.size() <= rank && state->stack.size() != 0) {
        // Get an item from the stack and create the path
        boost::shared_ptr<NbestStackItem> item = state->PopStack();
        boost::shared_ptr<HyperPath> act_path = NbestCreatePath(node, *item, uniq, states);
        // Check whether to add it or not
        if(uniq) {
            CfgDataVector trg = act_path->CalcTranslations();
            if(state->uniq_sents.find(trg) == state->uniq_sents.end()) {
                state->uniq_sents.insert(trg);
                state->paths.push_back(act_path);
            }
        } else {
            state->paths.push_back(act_path);
        }
        // Expand the stack by incrementing each path's child by one
        for(int i = 0; i < item->child_ranks.size(); i++) {
            boost::shared_ptr<NbestStackItem> next_item = NbestStackIncrementChild(node, *item, i, uniq, states);
            if(next_item.get() != NULL)
                state->PushStack(next_item);
        }
    }
    // Return the path if found, null if not
    if(state->paths.size() > rank)
        return state->paths[rank];
    else
        return boost::shared_ptr<HyperPath>();
}

vector<boost::shared_ptr<HyperPath> > HyperGraph::GetNbest(int n, bool uniq) {
    vector<boost::shared_ptr<NbestState> > states(nodes_.size());
    vector<boost::shared_ptr<HyperPath> > ret;
    for(int i = 0; i < n; i++) {
        boost::shared_ptr<HyperPath> path = NbestGetNthPath(0, i, uniq, states);
        if(path.get() == NULL) break;
        path->SetTrgData(path->CalcTranslations());
        ret.push_back(path);
    }
    return ret;
}


// Check to make sure that two hyperpaths are equal
bool HyperPath::operator==(const HyperPath & rhs) const {
    if(edges_.size() != rhs.edges_.size())
        return false;
    for(int i = 0; i < (int)edges_.size(); i++)
       if((edges_[i]==NULL) != (rhs.edges_[i]==NULL) ||
          (edges_[i]!=NULL && edges_[i]->GetId() != rhs.edges_[i]->GetId()))
          return false;
    return abs(score_ - rhs.score_) < 1e-5;
}

// Output for a hyperedge in JSON format
void HyperPath::Print(std::ostream & out) const {
    out << "{\"edges\": [";
    for(int i = 0; i < (int)edges_.size(); i++)
        out << (i != 0 ? ", " : "") << edges_[i]->GetId();
    out << "], \"score\": " << score_;
    out << "}";
}


// Calculate the features for this path by simply adding up all the features
SparseVector HyperPath::CalcFeatures() {
    SparseVector ret;
    BOOST_FOREACH(HyperEdge* edge, edges_)
        ret += edge->GetFeatures();
    return ret;
}

// Calculate the translation of the path
CfgData HyperPath::CalcTranslation(int factor, int & idx) {
    vector<CfgData> child_trans;
    int my_id = idx++;
    BOOST_FOREACH(HyperNode * tail, edges_[my_id]->GetTails()) {
        if(tail != edges_[idx]->GetHead())
            THROW_ERROR("Unmatching hyper-nodes " << *tail << " and " << *edges_[idx]->GetHead() << " at edge" << idx);
        child_trans.push_back(CalcTranslation(factor, idx));
    }
    CfgData ret;
    if(factor >= (int)edges_[my_id]->GetTrgData().size())
        return ret;
    BOOST_FOREACH(int wid, edges_[my_id]->GetTrgData()[factor].words) {
        if(wid >= 0) {
            ret.words.push_back(wid);
        } else {
            ret.AppendChild(child_trans[-1 - wid]);
        }
    }
    return ret;
}

// Score each edge in the graph
void HyperGraph::ScoreEdges(const Weights & weights) {
    BOOST_FOREACH(HyperEdge * edge, edges_)
        edge->SetScore(weights.GetCurrent() * edge->GetFeatures());
}

class QueueEntry {
public:
    QueueEntry(Real score, Real lm_score, const vector<int> & id) :
        score_(score), lm_score_(lm_score), id_(id) { }
    // Score of the entry
    Real score_, lm_score_;
    vector<int> id_;
};

void HyperEdge::SetRule(const TranslationRule * rule, const SparseVector & orig_features) {
    // src_str_ = rule->GetSrcStr();
    features_ = rule->GetFeatures() + orig_features;
    trg_data_ = rule->GetTrgData();
}

Real HyperNode::GetInsideProb(vector<Real> & inside) {
    if(inside[id_] != -REAL_MAX)
        return inside[id_];
    else if(IsTerminal())
        return (inside[id_] = 0);
    vector<Real> sum_over;
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        Real next = edge->GetScore();
        BOOST_FOREACH(HyperNode * tail, edge->GetTails())
            next += tail->GetInsideProb(inside);
        sum_over.push_back(next);
    }
    inside[id_] = AddLogProbs(sum_over);
    for(int i = 0; i < (int)sum_over.size(); i++) {
        edges_[i]->SetScore(sum_over[i] - inside[id_]);
    }
    return inside[id_];
}

Real HyperNode::GetOutsideProb(const vector< vector<HyperEdge*> > & all_edges, vector<Real> & outside) const {
    if(outside[id_] != -REAL_MAX)
        return outside[id_];
    else if(id_ == 0)
        return (outside[id_] = 0);
    else if(all_edges[id_].size() == 0)
        return (outside[id_] = -REAL_MAX);
    vector<Real> sum_over;
    BOOST_FOREACH(const HyperEdge * edge, all_edges[id_]) {
        Real next = edge->GetScore() + edge->GetHead()->GetOutsideProb(all_edges, outside);
        sum_over.push_back(next);
    }
    return (outside[id_] = AddLogProbs(sum_over));
}

vector< vector<HyperEdge*> > HyperGraph::GetReversedEdges() {
    vector< vector<HyperEdge*> > ret(nodes_.size());
    BOOST_FOREACH(HyperEdge* edge, edges_)
        BOOST_FOREACH(HyperNode* tail, edge->GetTails())
            ret[tail->GetId()].push_back(edge);
    return ret;
}

// Perform the inside-outside algorithm, where each edge score is a log probability
void HyperGraph::InsideOutsideNormalize() {
    // Calculate the inside and outside probabilities
    vector<Real> inside(nodes_.size(), -REAL_MAX), outside(nodes_.size(), -REAL_MAX);
    vector<vector<HyperEdge*> > rev_edges = GetReversedEdges();
    vector<Real> new_scores(edges_.size(), -REAL_MAX);
    nodes_[0]->GetInsideProb(inside);
    // Re-score the current edges
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        Real next = edge->GetScore() + edge->GetHead()->GetOutsideProb(rev_edges, outside);
        new_scores[edge->GetId()] = next;
    }
    for(int i = 0; i < (int)new_scores.size(); i++)
        edges_[i]->SetScore(new_scores[i]);
}

// Calculate new viterbi scores if necessary
Real HyperNode::CalcViterbiScore() {
    if(viterbi_score_ == -REAL_MAX) {
        // if(edges_.size() == 0)
        //     THROW_ERROR("Cannot GetViterbiScore for a node with no edges");
        BOOST_FOREACH(HyperEdge * edge, edges_) {
            Real score = edge->GetScore();
            BOOST_FOREACH(HyperNode * tail, edge->GetTails())
                score += tail->CalcViterbiScore();
            if(score > viterbi_score_)
                viterbi_score_ = score;
        }
    }
    return viterbi_score_;
}

// First copy the edges and nodes, then refresh the pointers
HyperGraph::HyperGraph(const HyperGraph & rhs) : words_(rhs.words_), edge_type_(rhs.edge_type_) {
    BOOST_FOREACH(HyperNode * node, rhs.nodes_)
        nodes_.push_back(new HyperNode(*node));
    edge_type_ = rhs.edge_type_;
    if(edge_type_ == HYPER_EDGE) {
        BOOST_FOREACH(HyperEdge * edge, rhs.edges_)
            edges_.push_back(new HyperEdge(*edge));
    } else if (edge_type_ == RULE_EDGE) {
        BOOST_FOREACH(HyperEdge * edge, rhs.edges_)
            edges_.push_back(new RuleEdge(*static_cast<RuleEdge*>(edge)));
    }
    BOOST_FOREACH(HyperNode * node, nodes_)
        node->RefreshPointers(*this);
    BOOST_FOREACH(HyperEdge * edge, edges_)
        edge->RefreshPointers(*this);
}
HyperGraph::~HyperGraph() {
    BOOST_FOREACH(HyperNode* node, nodes_)
        delete node;
    BOOST_FOREACH(HyperEdge* edge, edges_)
        delete edge;
};

void HyperGraph::DeleteNodes() {
    BOOST_FOREACH(HyperNode* node, nodes_)
        delete node;
    nodes_.resize(0);
}
void HyperGraph::DeleteEdges() {
    BOOST_FOREACH(HyperEdge* edge, edges_)
        delete edge;
    edges_.resize(0);
}


void HyperGraph::ResetViterbiScores() {
    BOOST_FOREACH(HyperNode * node, nodes_)
        node->SetViterbiScore(-REAL_MAX);
}

int HyperGraph::Append(const HyperGraph & rhs) {
    int node_start = nodes_.size();
    int edge_start = edges_.size();
    // Append the nodes
    BOOST_FOREACH(const HyperNode * node, rhs.GetNodes())
        nodes_.push_back(new HyperNode(*node));
    BOOST_FOREACH(const HyperEdge * edge, rhs.GetEdges())
        edges_.push_back(new HyperEdge(*edge));
    // Re-adjust the links
    for(int i = node_start; i < (int)nodes_.size(); i++) {
        nodes_[i]->SetId(i);
        vector<HyperEdge*> & node_edges = nodes_[i]->GetEdges();
        for(int j = 0; j < (int)node_edges.size(); j++)
            node_edges[j] = edges_[node_edges[j]->GetId()+edge_start];
    }
    for(int i = edge_start; i < (int)edges_.size(); i++) {
        edges_[i]->SetId(i);
        edges_[i]->SetHead(nodes_[edges_[i]->GetHead()->GetId()+node_start]);
        vector<HyperNode*> & edge_tails = edges_[i]->GetTails();
        for(int j = 0; j < (int)edge_tails.size(); j++)
            edge_tails[j] = nodes_[edge_tails[j]->GetId()+node_start];
    }
    // Return the node id
    return node_start;
}

// Adders. Add the value, and set its ID appropriately
// HyperGraph will take control of the added value
void HyperGraph::AddNode(HyperNode * node) {
    if(node->GetId() == -1) {
        node->SetId(nodes_.size());
        nodes_.push_back(node);
    } else {
        if((int)nodes_.size() <= node->GetId())
            nodes_.resize(node->GetId()+1, NULL);
        else if(nodes_[node->GetId()] != NULL)
            THROW_ERROR("Duplicate node addition @ " << node->GetId());
        nodes_[node->GetId()] = node;
    }
}
void HyperGraph::AddEdge(HyperEdge * edge) {
    edge->SetId(edges_.size());
    edges_.push_back(edge);
}
void HyperGraph::AddWord(WordId id) {
    words_.push_back(id);
}

LabeledSpans HyperGraph::GetLabeledSpans() const {
    LabeledSpans ret;
    // Do this in reverse so the earlier values over-write the later values.
    // This will have the effect of having nodes higher-up in the tree being
    // given precedence, as long as heads always come higher in the graph
    // than tails.
    BOOST_REVERSE_FOREACH(const HyperNode * node, nodes_)
        ret[node->GetSpan()] = node->GetSym();
    return ret;
}

CfgDataVector HyperPath::CalcTranslations() {
    if(edges_.size() == 0)
        THROW_ERROR("Cannot calculate a translation for a path with no edges");
    CfgDataVector ret(GlobalVars::trg_factors);
    for(int i = 0; i < GlobalVars::trg_factors; i++)
        ret[i] = CalcTranslation(i);
    return ret;
}

const HyperNode* HyperGraph::GetNode(int i) const { return nodes_[i]; }
HyperNode* HyperGraph::GetNode(int i) { return nodes_[i]; }
