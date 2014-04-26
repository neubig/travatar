#include <queue>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/tr1/unordered_set.hpp>
#include <lm/left.hh>
#include <lm/model.hh>
#include <travatar/weights.h>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>
#include <travatar/generic-string.h>
#include <travatar/util.h>
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
        for(int j = 0; j < (int)trg_data_.size(); j++) {
            const Sentence & trg_words = trg_data_[j].words;
            for(int i = 0; i < (int)trg_words.size(); i++) {
                // Handle pseudo-symbols
                if(trg_words[i] < 0)
                    out << trg_words[i];
                // Handle regular words
                else
                    out << '"' << Dict::EscapeString(Dict::WSym(trg_words[i])) << '"';
                out << ((i == (int)trg_words.size()-1) ? "]" : ", ");
            }
            out << ((j == (int)trg_data_.size()-1) ? "]" : ", ");
        }
    }
    if(features_.size())
        out << ", \"features\": " << features_;
    if(fragment_edges_.size()) {
        out << ", \"fragment_edges\": [";
        for(int i = 0; i < (int)fragment_edges_.size(); i++)
            out << fragment_edges_[i]->GetId() << ((i == (int)fragment_edges_.size()-1) ? "]" : ", ");
    }
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
    out << "{\"sym\": ";
    if(sym_==-1)
        out << "null";
    else 
        out << "\""<<Dict::WSymEscaped(sym_)<<"\"";
    out << ", \"span\": "<<src_span_<<", \"id\": "<<id_;
    if(edges_.size()) {
        out << ", \"edges\": [";
        for(int i = 0; i < (int)edges_.size(); i++)
            out << edges_[i]->GetId() << ((i == (int)edges_.size()-1) ? "]" : ", ");
    }
    if(trg_span_.size() > 0) {
        out << ", \"trg_span\": [";
        int num = 0;
        BOOST_FOREACH(int v, trg_span_)
            out << (num++ != 0?", ":"") << v;
        out << "]";
    }
    if(viterbi_score_ != -DBL_MAX)
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
    if(!(edges_.size() == rhs.edges_.size() &&
         nodes_.size() == rhs.nodes_.size() &&
         CheckVector(words_, rhs.words_))) {
        cerr << "edges: " << edges_.size() << " == " << rhs.edges_.size() << endl;
        cerr << "nodes: " << nodes_.size() << " == " << rhs.nodes_.size() << endl;
        return 0;
    }
    vector<double> l_scores(nodes_.size()), r_scores(nodes_.size());
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

class PathScoreMore {
public:
    bool operator()(const shared_ptr<HyperPath> x, const shared_ptr<HyperPath> y) {
        if(abs(x->GetScore() - y->GetScore()) > 1e-6) return x->GetScore() > y->GetScore();
        const vector<HyperEdge*> & xe = x->GetEdges(), ye = y->GetEdges();
        if(xe.size() != ye.size()) return xe.size() > ye.size();
        for(int i = 0; i < (int)xe.size(); i++)
            if(xe[i]->GetId() != ye[i]->GetId())
                return xe[i]->GetId() < ye[i]->GetId();
        return 0;
    }
};

vector<shared_ptr<HyperPath> > HyperGraph::GetNbest(int n, const std::vector<WordId> & src_words) {
    set<shared_ptr<HyperPath>, PathScoreMore> paths;
    shared_ptr<HyperPath> init_path(new HyperPath);
    init_path->PushNode(nodes_[0]);
    init_path->AddScore(nodes_[0]->CalcViterbiScore());
    // cerr << "Generating nbest, viterbi = " << nodes_[0]->CalcViterbiScore() << endl;
    paths.insert(init_path);
    vector<shared_ptr<HyperPath> > ret;
    while(paths.size() > 0 && (int)ret.size() < n) {
        shared_ptr<HyperPath> curr_path = *paths.begin();
        paths.erase(paths.begin());
        // cerr << " Processing " << *curr_path << endl;
        HyperNode * node = curr_path->PopNode();
        if(node == NULL) {
            curr_path->SetTrgData(curr_path->CalcTranslations());
            ret.push_back(curr_path);
        } else {
            curr_path->AddScore(-1*node->CalcViterbiScore());
            // Expand each different edge
            BOOST_FOREACH(HyperEdge * edge, node->GetEdges()) {
                // Create a new path that is a copy of the old one, and add
                // the edge and its corresponding score
                shared_ptr<HyperPath> next_path(new HyperPath(*curr_path));
                next_path->AddEdge(edge);
                next_path->AddScore(edge->GetScore());
                BOOST_FOREACH(HyperNode * tail_node, edge->GetTails())
                    next_path->AddScore(tail_node->CalcViterbiScore());
                // Add the nodes in reverse order, to ensure that we
                // are doing a depth-first left-to-right traversal
                BOOST_REVERSE_FOREACH(HyperNode * tail, edge->GetTails())
                    next_path->PushNode(tail);
                paths.insert(next_path);
            }
            while((int)paths.size() > n)
                paths.erase(boost::prior(paths.end()));
        }
        
    }
    return ret;
}


// Check to make sure that two hyperpaths are equal
bool HyperPath::operator==(const HyperPath & rhs) const {
    for(int i = 0; i < (int)edges_.size(); i++)
       if((edges_[i]==NULL) != (rhs.edges_[i]==NULL) ||
          (edges_[i]!=NULL && edges_[i]->GetId() != rhs.edges_[i]->GetId()))
          return false;
    for(int i = 0; i < (int)remaining_nodes_.size(); i++)
       if((remaining_nodes_[i]==NULL) != (rhs.remaining_nodes_[i]==NULL) ||
          (remaining_nodes_[i]!=NULL && remaining_nodes_[i]->GetId() != rhs.remaining_nodes_[i]->GetId()))
          return false;
    return abs(score_ - rhs.score_) < 1e-5;
}

// Output for a hyperedge in JSON format
void HyperPath::Print(std::ostream & out) const {
    out << "{\"edges\": [";
    for(int i = 0; i < (int)edges_.size(); i++)
        out << (i != 0 ? ", " : "") << edges_[i]->GetId();
    out << "], \"score\": " << score_;
    if(remaining_nodes_.size()) {
        out << ", \"remaining_nodes\": [";
        for(int i = 0; i < (int)remaining_nodes_.size(); i++)
            out << remaining_nodes_[i]->GetId() << ((i == (int)remaining_nodes_.size()-1) ? "]" : ", ");
    }
    out << "}";
}


// Calculate the features for this path by simply adding up all the features
SparseMap HyperPath::CalcFeatures() {
    SparseMap ret;
    BOOST_FOREACH(HyperEdge* edge, edges_)
        ret += edge->GetFeatures();
    return ret;
}

// Calculate the translation of the path
CfgData HyperPath::CalcTranslation(int factor, int & idx) {
    vector<CfgData> child_trans;
    int my_id = idx++;
    BOOST_FOREACH(HyperNode * tail, SafeAccess(edges_, my_id)->GetTails()) {
        if(tail != edges_[idx]->GetHead())
            THROW_ERROR("Unmatching hyper-nodes " << *tail);
        child_trans.push_back(CalcTranslation(factor, idx));
    }
    CfgData ret;
    BOOST_FOREACH(int wid, edges_[my_id]->GetTrgData()[factor].words) {
        // // Special handling of unknowns
        // if(wid == Dict::WID("<unk>")) {
        //     // For terminals, map all source words into the target
        //     if(edges_[my_id]->GetTails().size() == 0) {
        //         pair<int,int> span = edges_[my_id]->GetHead()->GetSpan();
        //         for(int i = span.first; i < span.second; i++)
        //             ret.push_back(src_words.size() == 0 ? wid : SafeAccess(src_words, i));
        //     // For non-terminals, map in order
        //     } else {
        //         BOOST_FOREACH(const vector<int> & vec, child_trans)
        //             BOOST_FOREACH(int next_wid, vec)
        //                 ret.push_back(next_wid);
        //     }
        // } else
        if(wid >= 0) {
            ret.words.push_back(wid);
        } else {
            ret.AppendChild(child_trans[-1 - wid]);
            // BOOST_FOREACH(int next_wid, child_trans[-1 - wid])
            //     ret.words.push_back(next_wid);
        }
    }
    return ret;
}

// Score each edge in the graph
void HyperGraph::ScoreEdges(const Weights & weights) {
    BOOST_FOREACH(HyperEdge * edge, edges_)
        edge->SetScore(weights * edge->GetFeatures());
}

class QueueEntry {
public:
    QueueEntry(double score, double lm_score, const GenericString<int> & id) :
        score_(score), lm_score_(lm_score), id_(id) { }
    // Score of the entry
    double score_, lm_score_;
    GenericString<int> id_;
};

inline string PrintContext(const State & context) {
    ostringstream out;
    out << "[";
    for(unsigned i = 0; i < context.length; i++) {
        if(i != 0) out << ", ";
        out << context.words[i];
    }
    out << "]";
    return out.str();
}
inline string PrintContext(const Left & context) {
    ostringstream out;
    out << "[";
    for(unsigned i = 0; i < context.length; i++) {
        if(i != 0) out << ", ";
        out << context.pointers[i];
    }
    out << "]";
    return out.str();
}

void HyperEdge::SetRule(const TranslationRule * rule, const SparseMap & orig_features) {
    rule_str_ = rule->GetSrcStr();
    features_ = rule->GetFeatures() + orig_features;
    trg_data_ = rule->GetTrgData();
}

double HyperNode::GetInsideProb(vector<double> & inside) {
    if(SafeAccess(inside, id_) != -DBL_MAX)
        return inside[id_];
    else if(IsTerminal())
        return (inside[id_] = 0);
    // cerr << "@node: " << *this << ": " << edges_.size() << endl;
    vector<double> sum_over;
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        // cerr << "   @edge: " << *edge << endl;
        double next = edge->GetScore();
        BOOST_FOREACH(HyperNode * tail, edge->GetTails())
            next += tail->GetInsideProb(inside);
        sum_over.push_back(next);
    }
    inside[id_] = AddLogProbs(sum_over);
    // cerr << "Inside over " << sum_over.size() << " edges=" << edges_.size() << " @ " << id_ << ": " << AddLogProbs(sum_over) << endl;
    for(int i = 0; i < (int)sum_over.size(); i++) {
        // cerr << "edges_["<<i<<"->SetScore("<<sum_over[i]<<" - " << inside[id_]<<")" << endl;
        edges_[i]->SetScore(sum_over[i] - inside[id_]);
    }
    return inside[id_];
}

double HyperNode::GetOutsideProb(const vector< vector<HyperEdge*> > & all_edges, vector<double> & outside) const {
    if(SafeAccess(outside, id_) != -DBL_MAX)
        return outside[id_];
    else if(id_ == 0)
        return (outside[id_] = 0);
    else if(all_edges[id_].size() == 0)
        return (outside[id_] = -DBL_MAX);
    vector<double> sum_over;
    BOOST_FOREACH(const HyperEdge * edge, all_edges[id_]) {
        double next = edge->GetScore() + edge->GetHead()->GetOutsideProb(all_edges, outside);
        sum_over.push_back(next);
    }
    // cerr << "Outside over " << sum_over.size() << " @ " << id_ << " : " << AddLogProbs(sum_over) << endl;
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
    vector<double> inside(nodes_.size(), -DBL_MAX), outside(nodes_.size(), -DBL_MAX);
    vector<vector<HyperEdge*> > rev_edges = GetReversedEdges();
    vector<double> new_scores(edges_.size(), -DBL_MAX);
    nodes_[0]->GetInsideProb(inside);
    // cerr << exp(nodes_[0]->GetInsideProb(inside)) << endl;
    // Re-score the current edges
    BOOST_FOREACH(HyperEdge * edge, edges_) {
        double next = edge->GetScore() + edge->GetHead()->GetOutsideProb(rev_edges, outside);
        // cerr << "next @ "<<edge->GetId()<<": " << edge->GetScore() << " + " << edge->GetHead()->GetOutsideProb(rev_edges, outside);
        new_scores[edge->GetId()] = next;
    }
    for(int i = 0; i < (int)new_scores.size(); i++)
        edges_[i]->SetScore(new_scores[i]);
}

// Calculate new viterbi scores if necessary
double HyperNode::CalcViterbiScore() {
    if(viterbi_score_ == -DBL_MAX) {
        // if(edges_.size() == 0)
        //     THROW_ERROR("Cannot GetViterbiScore for a node with no edges");
        BOOST_FOREACH(HyperEdge * edge, edges_) {
            double score = edge->GetScore();
            BOOST_FOREACH(HyperNode * tail, edge->GetTails())
                score += tail->CalcViterbiScore();
            if(score > viterbi_score_)
                viterbi_score_ = score;
        }
    }
    return viterbi_score_;
}

// First copy the edges and nodes, then refresh the pointers
HyperGraph::HyperGraph(const HyperGraph & rhs) {
    words_ = rhs.words_;
    BOOST_FOREACH(HyperNode * node, rhs.nodes_)
        nodes_.push_back(new HyperNode(*node));
    BOOST_FOREACH(HyperEdge * edge, rhs.edges_)
        edges_.push_back(new HyperEdge(*edge));
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
        node->SetViterbiScore(-DBL_MAX);
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
