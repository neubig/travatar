#ifndef TRAVATAR_HYPER_GRAPH__
#define TRAVATAR_HYPER_GRAPH__

#include <vector>
#include <boost/foreach.hpp>
#include <travatar/dict.h>

namespace travatar {

typedef short NodeId;
class HyperNode;
class HyperGraph;

// A hyperedge in the hypergraph
class HyperEdge {
protected:
    NodeId id_;
    HyperNode* head_;
    std::vector<HyperNode*> tails_;
    double score_;
    // A pointer to edges in a separate hypergraph that are
    // matched by a rule represented by this edge (for use in rule graphs)
    std::vector<HyperEdge*> fragment_edges_;
public:
    HyperEdge(HyperNode* head = NULL) : id_(-1), head_(head), score_(1) { };
    ~HyperEdge() { };

    // Adder
    void AddTail(HyperNode* tail) { tails_.push_back(tail); }
    void AddFragmentEdge(HyperEdge* edge) {
        fragment_edges_.push_back(edge);
        score_ *= edge->GetProb();
    }

    // Get the probability (score, and must be between 0 and 1
    double GetProb() const {
#ifdef TRAVATAR_SAFE
        if(!(score_ >= 0 && score_ <= 1))
            THROW_ERROR("Invalid probability "<<score_);
#endif
        return score_;
    }
    void SetProb(double score) { 
        score_ = score;
#ifdef TRAVATAR_SAFE
        if(!(score_ >= 0 && score_ <= 1))
            THROW_ERROR("Invalid probability "<<score_);
#endif
    }

    // Getters/Setters
    void SetId(NodeId id) { id_ = id; }
    NodeId GetId() const { return id_; }
    void SetHead(HyperNode* head) { head_ = head; }
    HyperNode* GetHead() const { return head_; }
    const std::vector<HyperNode*> & GetTails() const { return tails_; }
    std::vector<HyperNode*> & GetTails() { return tails_; }
    const std::vector<HyperEdge*> & GetFragmentEdges() const { return fragment_edges_; }
    std::vector<HyperEdge*> & GetFragmentEdges() { return fragment_edges_; }

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
    std::set<int>* trg_span_;
    // Whether or not this node is a frontier node
    FrontierType frontier_;
public:
    HyperNode(WordId sym = -1,
              std::pair<int,int> span = std::pair<int,int>(-1,-1),
              int id = -1) : 
        id_(id), sym_(sym), src_span_(span), trg_span_(NULL),
        frontier_(UNSET_FRONTIER) { };
    ~HyperNode() { };

    // Calculate the spans and frontiers using the GHKM algorithm
    const std::set<int> * CalculateTrgSpan(
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
    const std::set<int>* GetTrgSpan() const { return trg_span_; }
    std::set<int>* GetTrgSpan() { return trg_span_; }
    void SetTrgSpan(const std::set<int>& trg_span) { trg_span_ = new std::set<int>(trg_span); }
    FrontierType IsFrontier() const { return frontier_; }
    void SetFrontier(FrontierType frontier) { frontier_ = frontier; }

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

// The hypergraph
class HyperGraph {
protected:
    std::vector<HyperNode*> nodes_;
    std::vector<HyperEdge*> edges_;
    std::vector<WordId> words_;
public:

    HyperGraph() { };
    ~HyperGraph() {
        BOOST_FOREACH(HyperNode* node, nodes_)
            delete node;
        BOOST_FOREACH(HyperEdge* edge, edges_)
            delete edge;
    };

    // Check to make sure that the probabilities of edges
    // outgoing from a particular node add to one
    void NormalizeEdgeProbabilities() {
        BOOST_FOREACH(HyperNode* node, nodes_) {
            double sum = 0;
            BOOST_FOREACH(HyperEdge* edge, node->GetEdges())
                sum += edge->GetProb();
            BOOST_FOREACH(HyperEdge* edge, node->GetEdges())
                edge->SetProb(edge->GetProb() / sum);
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

    // Accessors
    const HyperNode* GetNode(int i) const { return SafeAccess(nodes_,i); }
    HyperNode* GetNode(int i) { return SafeAccess(nodes_,i); }
    const std::vector<HyperNode*> GetNodes() const { return nodes_; }
    std::vector<HyperNode*> GetNodes() { return nodes_; }
    int NumNodes() const { return nodes_.size(); }
    const HyperEdge* GetEdge(int i) const { return SafeAccess(edges_,i); }
    HyperEdge* GetEdge(int i) { return SafeAccess(edges_,i); }
    const std::vector<HyperEdge*> & GetEdges() const { return edges_; }
    int NumEdges() const { return edges_.size(); }
    const std::vector<WordId> & GetWords() const { return words_; }
    std::vector<WordId> & GetWords() { return words_; }
    void SetWords(const std::vector<WordId> & words) { words_ = words; }

};

}

#endif
