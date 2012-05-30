#ifndef TRABATAR_HYPER_GRAPH__
#define TRABATAR_HYPER_GRAPH__

#include <vector>
#include <boost/foreach.hpp>
#include <travatar/dict.h>

namespace travatar {

typedef short NodeId;
class HyperNode;

// A hyperedge in the hypergraph
class HyperEdge {
protected:
    NodeId id_;
    HyperNode* head_;
    std::vector<HyperNode*> tails_;
public:
    HyperEdge(HyperNode* head = NULL) : id_(-1), head_(head) { };
    ~HyperEdge() { };

    // Adder
    void AddTail(HyperNode* tail) { tails_.push_back(tail); }

    // Getters/Setters
    void SetId(NodeId id) { id_ = id; }
    NodeId GetId() { return id_; }

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
    friend std::ostream &operator<<( std::ostream &out, const HyperNode &L );
private:
    WordId sym_;
    std::pair<int,int> span_;
    NodeId id_;
    std::vector<HyperEdge*> edges_;
public:
    HyperNode(WordId sym,
              std::pair<int,int> span = std::pair<int,int>(-1,-1),
              int id = -1) : 
        sym_(sym), span_(span), id_(id) { };
    ~HyperNode() { };

    // Adders
    void AddEdge(HyperEdge* edge) { edges_.push_back(edge); }

    // Getters/Setters
    void SetId(NodeId id) { id_ = id; }
    NodeId GetId() { return id_; }
    const std::pair<int,int> & GetSpan() const { return span_; }
    std::pair<int,int> & GetSpan() { return span_; }
    int NumEdges() const { return edges_.size(); }
    const HyperEdge* GetEdge(int i) const { return SafeAccess(edges_, i); }
    HyperEdge* GetEdge(int i) { return SafeAccess(edges_, i); }

    // Operators
    bool operator==(const HyperNode & rhs) const;
    bool operator!=(const HyperNode & rhs) const {
        return !(*this == rhs);
    }

    // IO Functions
    void Print(std::ostream & out) const;

};
inline std::ostream &operator<<( std::ostream &out, const HyperNode &L ) {
    L.Print(out);
    return out;
}

// The hypergraph
class HyperGraph {
protected:
    int id_;
    std::vector<HyperNode*> nodes_;
    std::vector<HyperEdge*> edges_;
    std::vector<WordId> words_;
public:

    HyperGraph() : id_(-1) { };
    ~HyperGraph() {
        BOOST_FOREACH(HyperNode* node, nodes_)
            delete node;
        BOOST_FOREACH(HyperEdge* edge, edges_)
            delete edge;
    };

    // Check to make sure two hypergraphs are equal
    //  (print an error and return zero if not)
    int CheckEqual(const HyperGraph & rhs) const;

    // Adders. Add the value, and set its ID appropriately
    // HyperGraph will take control of the added value
    void AddNode(HyperNode * node) {
        node->SetId(nodes_.size());
        nodes_.push_back(node);
    }
    void AddEdge(HyperEdge * edge) {
        edge->SetId(edges_.size());
        edges_.push_back(edge);
    }

    // Accessors
    const HyperNode* GetNode(int i) const { return SafeAccess(nodes_,i); }
    HyperNode* GetNode(int i) { return SafeAccess(nodes_,i); }
    const HyperEdge* GetEdge(int i) const { return SafeAccess(edges_,i); }
    HyperEdge* GetEdge(int i) { return SafeAccess(edges_,i); }
    const std::vector<WordId> & GetWords() const { return words_; }
    std::vector<WordId> & GetWords() { return words_; }
    void SetWords(const std::vector<WordId> & words) { words_ = words; }

};

}

#endif
