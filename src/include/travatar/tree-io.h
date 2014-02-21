#ifndef TRAVATAR_TREE_IO__
#define TRAVATAR_TREE_IO__

#include <vector>
#include <iostream>
#include "travatar/sentence.h"

namespace travatar {

class HyperGraph;
class HyperNode;
template <class T>
class SymbolSet;

// A virtual class to read in and write out parse trees
class TreeIO {
public:
    virtual ~TreeIO() { };
    virtual HyperGraph * ReadTree(std::istream & in) = 0;
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out) = 0;
};

// Read in and write out Penn Treebank format trees
class PennTreeIO : public TreeIO {
public:
    virtual ~PennTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    void WriteNode(const std::vector<WordId> & words,
                   const HyperNode & node, std::ostream & out);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
};

// Read in and write out trees in the same format as travatar rules
class RuleTreeIO : public TreeIO {
public:
    virtual ~RuleTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    void WriteNode(const std::vector<WordId> & words,
                   const HyperNode & node, std::ostream & out);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
};

// Read in and write out only the words of trees
class WordTreeIO : public TreeIO {
public:
    virtual ~WordTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    void WriteNode(const std::vector<WordId> & words,
                   const HyperNode & node, std::ostream & out);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
};

// Read in and write out JSON format hypergraphs
class JSONTreeIO : public TreeIO {
public:
    virtual ~JSONTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
};

// Read in and write the format of the Egret parser
class EgretTreeIO : public TreeIO {
public:
    virtual ~EgretTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
private:
    HyperNode * MakeEgretNode(const std::string & str_id, SymbolSet<int> & node_map, HyperGraph * graph);
};

// Read in and write Moses XML tree format
class MosesXMLTreeIO : public TreeIO {
public:
    virtual ~MosesXMLTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
    void WriteNode(const std::vector<WordId> & words,
                   const HyperNode & node, std::ostream & out);
private:
};

}

#endif
