#ifndef TRAVATAR_HYPER_GRAPH__
#define TRAVATAR_HYPER_GRAPH__

#include <vector>
#include <climits>
#include <cfloat>
#include <boost/foreach.hpp>
#include <travatar/dict.h>
#include <lm/left.hh>

namespace travatar {

typedef short NodeId;
class HyperNode;
class HyperGraph;
class TranslationRule;
typedef std::vector<HyperNode*> ChartEntry;

// A hyperedge in the hypergraph
class HyperEdge {
protected:
    NodeId id_;
    HyperNode* head_;
    std::vector<HyperNode*> tails_;
    double score_;
    std::string rule_str_;
    std::vector<WordId> trg_words_;
    SparseMap features_;
    // A pointer to edges in a separate hypergraph that are
    // matched by a rule represented by this edge (for use in rule graphs)
    std::vector<HyperEdge*> fragment_edges_;
public:
    HyperEdge(HyperNode* head = NULL) : id_(-1), head_(head), score_(0.0) { };
    ~HyperEdge() { };

    // Refresh the pointers to head and tail nodes so they point to
    // nodes in a new HyperGraph. Useful when copying edges
    void RefreshPointers(HyperGraph & new_graph);

    // Adder
    void AddTail(HyperNode* tail) { tails_.push_back(tail); }
    void AddFragmentEdge(HyperEdge* edge) {
        fragment_edges_.push_back(edge);
        score_ += edge->GetScore();
    }   

    // Get the probability (score, and must be between 0 and 1
    double GetScore() const { return score_; }
    void SetScore(double score) { score_ = score; }

    // Getters/Setters
    void SetId(NodeId id) { id_ = id; }
    NodeId GetId() const { return id_; }
    void SetHead(HyperNode* head) { head_ = head; }
    HyperNode* GetHead() const { return head_; }
    const HyperNode* GetTail(int i) const { return tails_[i]; }
    HyperNode* GetTail(int i) { return tails_[i]; }
    const std::vector<HyperNode*> & GetTails() const { return tails_; }
    std::vector<HyperNode*> & GetTails() { return tails_; }
    void SetTails(const std::vector<HyperNode*> & tails) { tails_ = tails; }
    const std::vector<HyperEdge*> & GetFragmentEdges() const { return fragment_edges_; }
    std::vector<HyperEdge*> & GetFragmentEdges() { return fragment_edges_; }
    // const TranslationRule * GetRule() const { return rule_; }
    void SetRule(const TranslationRule * rule);
    const std::string & GetRuleStr() const { return rule_str_; }
    const std::vector<WordId> & GetTrgWords() const { return trg_words_; }
    const SparseMap & GetFeatures() const { return features_; }
    std::string & GetRuleStr() { return rule_str_; }
    std::vector<WordId> & GetTrgWords() { return trg_words_; }
    SparseMap & GetFeatures() { return features_; }

    // Operators
    bool operator==(const HyperEdge & rhs) const;
    bool operator!=(const HyperEdge & rhs) const {
        return !(*this == rhs);
    }

    // Input/Output
    void Print(std::ostream & out) const;

};
inline std::ostream &operator<<( std::ostream &out, const HyperEdge &L ) {
    L.Print(out);
    return out;
}

// A hypernode in the hypergraph
class HyperNode {
public:
    friend class HyperGraph;
    typedef enum {
        IS_FRONTIER = 'Y',
        NOT_FRONTIER = 'N',
        UNSET_FRONTIER = 'U'
    } FrontierType;
private:
    // The ID of the node in the hypergraph
    NodeId id_;
    // The ID of the non-terminal or terminal represented by this node
    WordId sym_;
    // The span in the source sentence covered
    std::pair<int,int> src_span_;
    // HyperEdges to child nodes
    std::vector<HyperEdge*> edges_;
    // For use in rule extraction, the span in the target sentence that this
    // node covers
    bool has_trg_span_;
    std::set<int> trg_span_;
    // Whether or not this node is a frontier node
    FrontierType frontier_;
    // The viterbi score of the entire subtree under this node
    double viterbi_score_;
public:
    HyperNode(WordId sym = -1,
              std::pair<int,int> span = std::pair<int,int>(-1,-1),
              int id = -1) : 
        id_(id), sym_(sym), src_span_(span), has_trg_span_(false),
        frontier_(UNSET_FRONTIER), viterbi_score_(-DBL_MAX) { };
    ~HyperNode() { };

    // Refresh the pointers to head and tail nodes so they point to
    // nodes in a new HyperGraph. Useful when copying nodes
    void RefreshPointers(HyperGraph & new_graph);

    void SetViterbiScore(double viterbi_score) {
        viterbi_score_ = viterbi_score;
    }
    double GetViterbiScore() {
        if(viterbi_score_ == -DBL_MAX) {
            if(edges_.size() == 0)
                THROW_ERROR("Cannot GetViterbiScore for a node with no edges");
            BOOST_FOREACH(HyperEdge * edge, edges_) {
                double score = edge->GetScore();
                BOOST_FOREACH(HyperNode * tail, edge->GetTails())
                    score += tail->GetViterbiScore();
                if(score > viterbi_score_) {
                    viterbi_score_ = score;
                }
            }
        }
        return viterbi_score_;
    }

    // Calculate the spans and frontiers using the GHKM algorithm
    const std::set<int> & CalculateTrgSpan(
            const std::vector<std::set<int> > & word_spans);

    // Information
    int NumEdges() const { return edges_.size(); }
    bool IsTerminal() const { return edges_.size() == 0; }

    // Adders
    void AddEdge(HyperEdge* edge) { edges_.push_back(edge); }

    // Getters/Setters
    void SetSym(WordId sym) { sym_ = sym; }
    WordId GetSym() const { return sym_; }
    void SetId(NodeId id) { id_ = id; }
    NodeId GetId() const { return id_; }
    const std::pair<int,int> & GetSpan() const { return src_span_; }
    std::pair<int,int> & GetSpan() { return src_span_; }
    void SetSpan(const std::pair<int,int> & span) { src_span_ = span; }
    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    std::vector<HyperEdge*> & GetEdges() { return edges_; }
    const HyperEdge* GetEdge(int i) const { return SafeAccess(edges_, i); }
    HyperEdge* GetEdge(int i) { return SafeAccess(edges_, i); }
    HyperNode::FrontierType GetFrontier() const { return frontier_; }
    bool HasTrgSpan() const { return has_trg_span_; }
    const std::set<int> & GetTrgSpan() const { return trg_span_; }
    std::set<int> & GetTrgSpan() { return trg_span_; }
    void SetTrgSpan(const std::set<int>& trg_span) { trg_span_ = trg_span; has_trg_span_ = true; }
    FrontierType IsFrontier() const { return frontier_; }
    void SetFrontier(FrontierType frontier) { frontier_ = frontier; }
    
    // Return the parts of the target sentence covered by this node
    // If this is the root node, it will cover the whole target sentence, and this 
    // will return <0, INT_MAX>
    // If this covers no part of the target, return <-1, -1>
    // Otherwise return the exact part that is actually covered
    std::pair<int, int> GetTrgCovered() {
        if(id_ == 0) return MakePair(0, INT_MAX);
        if(trg_span_.size() == 0) return MakePair(-1,-1);
        return MakePair(*trg_span_.begin(), *trg_span_.rbegin()+1);
    }

    // Operators
    bool operator==(const HyperNode & rhs) const;
    bool operator!=(const HyperNode & rhs) const {
        return !(*this == rhs);
    }

    // IO Functions
    void Print(std::ostream & out) const;

protected:
    // Create a frontier
    FrontierType CalculateFrontier(
                   const std::vector<std::set<int> > & src_spans,
                   const std::set<int> & complement);

};
inline std::ostream &operator<<( std::ostream &out, const HyperNode &L ) {
    L.Print(out);
    return out;
}

// A single scored path through a hypergraph
class HyperPath {
public:
    HyperPath() : score_(0) { }
    
    void AddEdge(HyperEdge * edge) { edges_.push_back(edge); }
    void PushNode(HyperNode * node) { remaining_nodes_.push_back(node); }
    HyperNode* PopNode() {
        HyperNode * ret = NULL;
        if(remaining_nodes_.size() > 0) {
            ret = *remaining_nodes_.rbegin();
            remaining_nodes_.pop_back();
        }
        return ret;
    }
    void SetScore(double score) { score_ = score; }
    double AddScore(double score) { return (score_ += score); }
    double GetScore() { return score_; }

    std::vector<WordId> CalcTranslation(const std::vector<WordId> & src_words) { 
        int idx = 0; return CalcTranslation(idx, src_words);
    }
    std::vector<WordId> CalcTranslation(int & idx, const std::vector<WordId> & src_words);

    // Calculate the features for this path by simply adding up all the features
    SparseMap CalcFeatures();

    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    std::vector<HyperEdge*> & GetEdges() { return edges_; }
    const HyperEdge* GetEdge(int i) const { return SafeAccess(edges_, i); }
    HyperEdge* GetEdge(int i) { return SafeAccess(edges_, i); }

    bool operator==(const HyperPath & rhs) const;
    bool operator!=(const HyperPath & rhs) const { return !(*this == rhs); }
    void Print(std::ostream & out) const;

protected:
    std::vector<HyperEdge*> edges_;
    double score_;
    // For use with partial paths, which nodes are still open?
    std::vector<HyperNode*> remaining_nodes_;
};
inline std::ostream &operator<<( std::ostream &out, const HyperPath &L ) {
    L.Print(out);
    return out;
}

// The hypergraph
class HyperGraph {
protected:
    std::vector<HyperNode*> nodes_;
    std::vector<HyperEdge*> edges_;
    std::vector<WordId> words_;
public:

    HyperGraph() { };
    // First copy the edges and nodes, then refresh the pointers
    HyperGraph(const HyperGraph & rhs) {
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
    ~HyperGraph() {
        BOOST_FOREACH(HyperNode* node, nodes_)
            delete node;
        BOOST_FOREACH(HyperEdge* edge, edges_)
            delete edge;
    };

    // Score each edge in the graph
    void ScoreEdges(const SparseMap & weights);

    // Get the n-best paths through the graph
    std::vector<boost::shared_ptr<HyperPath> > GetNbest(int n);

    // Check to make sure that the probabilities of edges
    // outgoing from a particular node add to one (in the log domain)
    void NormalizeEdgeProbabilities() {
        BOOST_FOREACH(HyperNode* node, nodes_) {
            double sum = 0;
            BOOST_FOREACH(HyperEdge* edge, node->GetEdges())
                sum += exp(edge->GetScore());
            sum = log(sum);
            BOOST_FOREACH(HyperEdge* edge, node->GetEdges())
                edge->SetScore(edge->GetScore() - sum);
        }
    }

    // Calculate the frontier for the whole graph
    void CalculateFrontiers(const std::vector<std::set<int> > & src_spans) {
        nodes_[0]->CalculateFrontier(src_spans, std::set<int>());
    }

    // Check to make sure two hypergraphs are equal
    //  (print an error and return zero if not)
    int CheckEqual(const HyperGraph & rhs) const;

    // Adders. Add the value, and set its ID appropriately
    // HyperGraph will take control of the added value
    void AddNode(HyperNode * node) {
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
    void AddEdge(HyperEdge * edge) {
        edge->SetId(edges_.size());
        edges_.push_back(edge);
    }
    void AddWord(WordId id) {
        words_.push_back(id);
    }

    void ResetViterbiScores() {
        BOOST_FOREACH(HyperNode * node, nodes_)
            node->SetViterbiScore(-DBL_MAX);
    }

    // Intersect this graph with a language model, using cube pruning to control
    // the overall state space.
    HyperGraph * IntersectWithLM(const lm::ngram::Model & lm, double lm_weight, int stack_pop_limit = INT_MAX);

    const ChartEntry & BuildChart(
                        const lm::ngram::Model & lm, 
                        double lm_weight,
                        int stack_pop_limit, 
                        std::vector<boost::shared_ptr<ChartEntry> > & chart, 
                        std::vector<lm::ngram::ChartState> & states, 
                        int id,
                        HyperGraph & graph);

    // Accessors
    const HyperNode* GetNode(int i) const { return SafeAccess(nodes_,i); }
    HyperNode* GetNode(int i) { return SafeAccess(nodes_,i); }
    const std::vector<HyperNode*> & GetNodes() const { return nodes_; }
    std::vector<HyperNode*> & GetNodes() { return nodes_; }
    int NumNodes() const { return nodes_.size(); }
    const HyperEdge* GetEdge(int i) const { return SafeAccess(edges_,i); }
    HyperEdge* GetEdge(int i) { return SafeAccess(edges_,i); }
    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    int NumEdges() const { return edges_.size(); }
    WordId GetWord(int i) const { return SafeAccess(words_, i); }
    const std::vector<WordId> & GetWords() const { return words_; }
    std::vector<WordId> & GetWords() { return words_; }
    void SetWords(const std::vector<WordId> & words) { words_ = words; }

};

}

#endif
