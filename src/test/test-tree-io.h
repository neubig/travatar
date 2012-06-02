#ifndef TEST_TREE_IO_H__
#define TEST_TREE_IO_H__

#include "test-base.h"
#include <travatar/tree-io.h>
#include <travatar/dict.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestTreeIO : public TestBase {

public:

    TestTreeIO() { }
    ~TestTreeIO() { }

    int TestReadPenn() {
        // Use this tree
        string tree = " (A (B (C x) (D y))\n (E z))AAA";
        stringstream instr;
        instr << tree;
        PennTreeIO io;
        boost::scoped_ptr<HyperGraph> hg_act(io.ReadTree(instr));
        // Create the words
        HyperGraph hg_exp; 
        HyperNode* a_node = new HyperNode(Dict::WID("A"), MakePair(0,3)); hg_exp.AddNode(a_node);
        HyperNode* b_node = new HyperNode(Dict::WID("B"), MakePair(0,2)); hg_exp.AddNode(b_node);
        HyperNode* c_node = new HyperNode(Dict::WID("C"), MakePair(0,1)); hg_exp.AddNode(c_node);
        HyperNode* x_node = new HyperNode(Dict::WID("x"), MakePair(0,1)); hg_exp.AddNode(x_node);
        HyperNode* d_node = new HyperNode(Dict::WID("D"), MakePair(1,2)); hg_exp.AddNode(d_node);
        HyperNode* y_node = new HyperNode(Dict::WID("y"), MakePair(1,2)); hg_exp.AddNode(y_node);
        HyperNode* e_node = new HyperNode(Dict::WID("E"), MakePair(2,3)); hg_exp.AddNode(e_node);
        HyperNode* z_node = new HyperNode(Dict::WID("z"), MakePair(2,3)); hg_exp.AddNode(z_node);
        hg_exp.SetWords(Dict::ParseWords("x y z"));
        // Make the edge for A
        HyperEdge * a_edge = new HyperEdge(a_node);
        a_edge->AddTail(b_node); a_edge->AddTail(e_node);
        a_node->AddEdge(a_edge); hg_exp.AddEdge(a_edge);
        // Make the edge for B
        HyperEdge * b_edge = new HyperEdge(b_node);
        b_edge->AddTail(c_node); b_edge->AddTail(d_node);
        b_node->AddEdge(b_edge); hg_exp.AddEdge(b_edge);
        // Make the edge for C
        HyperEdge * c_edge = new HyperEdge(c_node);
        c_edge->AddTail(x_node);
        c_node->AddEdge(c_edge); hg_exp.AddEdge(c_edge);
        // Make the edge for D
        HyperEdge * d_edge = new HyperEdge(d_node);
        d_edge->AddTail(y_node);
        d_node->AddEdge(d_edge); hg_exp.AddEdge(d_edge);
        // Make the edge for E
        HyperEdge * e_edge = new HyperEdge(e_node);
        e_edge->AddTail(z_node);
        e_node->AddEdge(e_edge); hg_exp.AddEdge(e_edge);
        // Check to make sure that the remaining values still remain
        string left_act; instr >> left_act;
        string left_exp = "AAA";
        // Check that both values are equal
        return hg_exp.CheckEqual(*hg_act) && left_act == left_exp;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestReadPenn()" << endl; if(TestReadPenn()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTreeIO Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
