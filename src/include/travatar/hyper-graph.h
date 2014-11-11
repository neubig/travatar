#ifndef TRAVATAR_HYPER_GRAPH__
#define TRAVATAR_HYPER_GRAPH__

#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <travatar/cfg-data.h>
#include <travatar/nbest-list.h>
#include <vector>
#include <climits>
#include <cfloat>
#include <set>
#include <map>

namespace travatar {

typedef int NodeId;
class HyperNode;
class HyperGraph;
class TranslationRule;
class Weights;
class RuleEdge;

typedef std::pair< std::pair<int,int>, WordId > LabeledSpan;
typedef std::map< std::pair<int,int>, WordId > LabeledSpans;

// A hyperedge in the hypergraph
class HyperEdge {
public:
    friend class RuleEdge;
protected:
    NodeId id_;
    HyperNode* head_;
    std::vector<HyperNode*> tails_;
    double score_;
    std::string src_str_;
    CfgDataVector trg_data_;
    SparseVector features_;
public:
    HyperEdge(HyperNode* head = NULL) : id_(-1), head_(head), score_(0.0) { };
    virtual ~HyperEdge() { };

    // Refresh the pointers to head and tail nodes so they point to
    // nodes in a new HyperGraph. Useful when copying edges
    void RefreshPointers(HyperGraph & new_graph);

    // Adder
    void AddTail(HyperNode* tail);
    virtual void AddFragmentEdge(HyperEdge* edge) {
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
    int NumTails() const { return tails_.size(); }
    void SetTails(const std::vector<HyperNode*> & tails) { tails_ = tails; }
    // const TranslationRule * GetRule() const { return rule_; }
    // Set the translation rule, including the features in the edges covered by the rule
    void SetRule(const TranslationRule * rule, const SparseVector & orig_features = SparseVector());
    const std::string & GetSrcStr() const { return src_str_; }
    const CfgDataVector & GetTrgData() const { return trg_data_; }
    const SparseVector & GetFeatures() const { return features_; }
    std::string & GetSrcStr() { return src_str_; }
    CfgDataVector & GetTrgData() { return trg_data_; }
    SparseVector & GetFeatures() { return features_; }
    void SetSrcStr(const std::string & str) { src_str_ = str; }
    void SetTrgData(const CfgDataVector & trg) { trg_data_ = trg; }
    void SetFeatures(const SparseVector & feat) { features_ = feat; }
    // void AddFeature(int idx, double feat) { features_[idx] += feat; }
    void AddTrgWord(int idx, int factor = 0) {
        if((int)trg_data_.size() <= factor)
            trg_data_.resize(factor+1);
        trg_data_[factor].words.push_back(idx);
    }

    // Operators
    virtual bool operator==(const HyperEdge & rhs) const;
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

class RuleEdge : public HyperEdge {

protected:
    // A pointer to edges in a separate hypergraph that are
    // matched by a rule represented by this edge (for use in rule graphs)
    std::vector<HyperEdge*> fragment_edges_;

public:
    RuleEdge(HyperNode* head = NULL) : HyperEdge(head) { }
    virtual ~RuleEdge() { }

    virtual void AddFragmentEdge(HyperEdge* edge) {
        fragment_edges_.push_back(edge);
        score_ += edge->GetScore();
    }   
    
    virtual bool operator==(const RuleEdge & rhs) const;
    bool operator!=(const RuleEdge & rhs) const {
        return !(*this == rhs);
    }

    const std::vector<HyperEdge*> & GetFragmentEdges() const { return fragment_edges_; }
    std::vector<HyperEdge*> & GetFragmentEdges() { return fragment_edges_; }
    void SetFragmentEdges(const std::vector<HyperEdge*> & fragment_edges) { fragment_edges_ = fragment_edges; }
};

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
    // The ID of the symbol represented on the target side
    WordId trg_sym_;
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
              WordId trg_sym = -1,
              std::pair<int,int> span = std::pair<int,int>(-1,-1),
              int id = -1) : 
        id_(id), sym_(sym), trg_sym_(trg_sym), src_span_(span), has_trg_span_(false),
        frontier_(UNSET_FRONTIER), viterbi_score_(-DBL_MAX) { };
    ~HyperNode() { };

    // Refresh the pointers to head and tail nodes so they point to
    // nodes in a new HyperGraph. Useful when copying nodes
    void RefreshPointers(HyperGraph & new_graph);
    
    // Set or get the viterbi score without re-calculating it
    void SetViterbiScore(double viterbi_score) { viterbi_score_ = viterbi_score; }
    double GetViterbiScore() const { return viterbi_score_; }
    // Calculate new viterbi scores if necessary
    double CalcViterbiScore();

    // Calculate the spans and frontiers using the GHKM algorithm
    const std::set<int> & CalculateTrgSpan(
            const std::vector<std::set<int> > & word_spans);

    // Information
    int NumEdges() const { return edges_.size(); }
    bool IsTerminal() const { return edges_.size() == 0; }
    bool IsPreTerminal() const { return edges_.size() == 1 && edges_[0]->GetTails().size() == 1 && edges_[0]->GetTail(0)->IsTerminal(); }

    // Adders
    void AddEdge(HyperEdge* edge) { edges_.push_back(edge); }

    // Remover
    void RemoveEdge(int position) { edges_.erase(edges_.begin() + position); }

    // Functions for the inside-outside algorithm
    double GetInsideProb(std::vector<double> & inside);
    double GetOutsideProb(const std::vector< std::vector<HyperEdge*> > & all_edges, std::vector<double> & outside) const;

    // Getters/Setters
    void SetSym(WordId sym) { sym_ = sym; }
    WordId GetSym() const { return sym_; }
    void SetTrgSym(WordId sym) { trg_sym_ = sym; }
    WordId GetTrgSym() const { return trg_sym_; }
    void SetId(NodeId id) { id_ = id; }
    NodeId GetId() const { return id_; }
    const std::pair<int,int> & GetSpan() const { return src_span_; }
    std::pair<int,int> & GetSpan() { return src_span_; }
    void SetSpan(const std::pair<int,int> & span) { src_span_ = span; }
    void AddSpan(const std::pair<int,int> & span) {
        if(src_span_.second == -1) src_span_ = span;
        else {
            src_span_.first = std::min(src_span_.first, span.first);
            src_span_.second = std::max(src_span_.second, span.second);
        }
    }
    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    std::vector<HyperEdge*> & GetEdges() { return edges_; }
    const HyperEdge* GetEdge(int i) const { return edges_[i]; }
    HyperEdge* GetEdge(int i) { return edges_[i]; }
    void SetEdges(const std::vector<HyperEdge*> edges) {
        edges_ = edges;
    }
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
    std::pair<int, int> GetTrgCovered() const {
        if(id_ == 0) return std::make_pair(0, INT_MAX);
        if(trg_span_.size() == 0) return std::make_pair(-1,-1);
        return std::make_pair(*trg_span_.begin(), *trg_span_.rbegin()+1);
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
    HyperPath() : edges_(), data_(), score_(0), loss_(0), remaining_nodes_() { }
    
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
    void SetLoss(double loss) { loss_ = loss; }
    double GetLoss() { return loss_; }

    CfgDataVector CalcTranslations();
    CfgData CalcTranslation(int factor) {
        int idx = 0; return CalcTranslation(factor, idx);
    }
    CfgData CalcTranslation(int factor, int & idx);

    // Calculate the features for this path by simply adding up all the features
    SparseVector CalcFeatures();

    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    std::vector<HyperEdge*> & GetEdges() { return edges_; }
    const HyperEdge* GetEdge(int i) const { return edges_[i]; }
    HyperEdge* GetEdge(int i) { return edges_[i]; }
    int NumEdges() const { return edges_.size(); }
    const CfgDataVector & GetTrgData() const { return data_; }
    CfgDataVector & GetTrgData() { return data_; }
    void SetTrgData(const CfgDataVector & data) { data_ = data; }
    SparseVector GetFeatures();

    bool operator==(const HyperPath & rhs) const;
    bool operator!=(const HyperPath & rhs) const { return !(*this == rhs); }
    void Print(std::ostream & out) const;

protected:
    // The edges contrained in this translation
    std::vector<HyperEdge*> edges_;
    // The actual translations themselves
    CfgDataVector data_;
    // The model score of the translation
    double score_;
    // The loss of the translation
    double loss_;
    // For use with partial paths, which nodes are still open?
    std::vector<HyperNode*> remaining_nodes_;
};
inline std::ostream &operator<<( std::ostream &out, const HyperPath &L ) {
    L.Print(out);
    return out;
}

// The hypergraph
class HyperGraph {
public: 
    typedef enum {
        HYPER_EDGE = 'H',
        RULE_EDGE = 'R'
    } EdgeType;
protected:
    std::vector<HyperNode*> nodes_;
    std::vector<HyperEdge*> edges_;
    Sentence words_;
    EdgeType edge_type_;
public:

    HyperGraph() : edge_type_(HYPER_EDGE) { };
    // First copy the edges and nodes, then refresh the pointers
    HyperGraph(const HyperGraph & rhs);
    ~HyperGraph();

    void DeleteNodes();
    void DeleteEdges();

    // Score each edge in the graph
    void ScoreEdges(const Weights & weights);

    // Get the n-best paths through the graph
    NbestList GetNbest(int n);

    // Calculate frontier nodes and alignments for the whole graph
    void CalculateFrontiers(const std::vector<std::set<int> > & src_spans) {
        nodes_[0]->CalculateFrontier(src_spans, std::set<int>());
    }

    // Append one hypergraph to another and return the root node of the
    // appended graph
    int Append(const HyperGraph & rhs);

    // Check to make sure two hypergraphs are equal
    //  (print an error and return zero if not)
    int CheckEqual(const HyperGraph & rhs) const;

    // Check if hypergraphs are maybe equal
    //  This is insensitive to ordering of nodes, and useful for lightly
    //  testing algorithms that use hashing that may change across
    //  architectures
    int CheckMaybeEqual(const HyperGraph & rhs) const;

    // Adders. Add the value, and set its ID appropriately
    // HyperGraph will take control of the added value
    void AddNode(HyperNode * node);
    void AddEdge(HyperEdge * edge);
    void AddWord(WordId id);

    void ResetViterbiScores();

    // Perform the inside-outside algorithm
    std::vector< std::vector<HyperEdge*> > GetReversedEdges();
    void InsideOutsideNormalize();

    // Get labeled spans
    LabeledSpans GetLabeledSpans() const;

    // Accessors
    const HyperNode* GetNode(int i) const;
    HyperNode* GetNode(int i);
    const std::vector<HyperNode*> & GetNodes() const { return nodes_; }
    std::vector<HyperNode*> & GetNodes() { return nodes_; }
    int NumNodes() const { return nodes_.size(); }
    const HyperEdge* GetEdge(int i) const { return edges_[i]; }
    HyperEdge* GetEdge(int i) { return edges_[i]; }
    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    std::vector<HyperEdge*> & GetEdges() { return edges_; }
    int NumEdges() const { return edges_.size(); }
    WordId GetWord(int i) const { return words_[i]; }
    const Sentence & GetWords() const { return words_; }
    Sentence & GetWords() { return words_; }
    void SetWords(const Sentence & words) { words_ = words; }
    EdgeType GetEdgeType() const { return edge_type_; }
    void SetEdgeType(EdgeType edge_type) { edge_type_ = edge_type; }

};

}

#endif
