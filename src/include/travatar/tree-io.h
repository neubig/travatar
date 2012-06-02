#ifndef TRABATAR_TREE_IO__
#define TRABATAR_TREE_IO__

#include <vector>
#include <iostream>
#include <sstream>
#include <travatar/hyper-graph.h>

namespace travatar {

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
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
};

// Read in and write out JSON Treebank format trees
class JSONTreeIO : public TreeIO {
public:
    virtual ~JSONTreeIO() { }
    virtual HyperGraph * ReadTree(std::istream & in);
    virtual void WriteTree(const HyperGraph & tree, std::ostream & out);
};

}

#endif
