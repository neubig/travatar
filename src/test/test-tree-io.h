#ifndef TEST_TREE_IO_H__
#define TEST_TREE_IO_H__

#include "test-base.h"
#include <travatar/tree-io.h>
#include <travatar/dict.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestTreeIO : public TestBase {

public:

    TestTreeIO() {
        tree_str = " (A (B (C x) (D y))\n (E z))AAA";
        // Create the words
        HyperNode* a_node = new HyperNode(Dict::WID("A"), -1, make_pair(0,3)); tree_exp.AddNode(a_node);
        HyperNode* b_node = new HyperNode(Dict::WID("B"), -1, make_pair(0,2)); tree_exp.AddNode(b_node);
        HyperNode* c_node = new HyperNode(Dict::WID("C"), -1, make_pair(0,1)); tree_exp.AddNode(c_node);
        HyperNode* x_node = new HyperNode(Dict::WID("x"), -1, make_pair(0,1)); tree_exp.AddNode(x_node);
        HyperNode* d_node = new HyperNode(Dict::WID("D"), -1, make_pair(1,2)); tree_exp.AddNode(d_node);
        HyperNode* y_node = new HyperNode(Dict::WID("y"), -1, make_pair(1,2)); tree_exp.AddNode(y_node);
        HyperNode* e_node = new HyperNode(Dict::WID("E"), -1, make_pair(2,3)); tree_exp.AddNode(e_node);
        HyperNode* z_node = new HyperNode(Dict::WID("z"), -1, make_pair(2,3)); tree_exp.AddNode(z_node);
        tree_exp.SetWords(Dict::ParseWords("x y z"));
        // Make the edge for A
        HyperEdge * a_edge = new HyperEdge(a_node);
        a_edge->AddTail(b_node); a_edge->AddTail(e_node);
        a_node->AddEdge(a_edge); tree_exp.AddEdge(a_edge);
        // Make the edge for B
        HyperEdge * b_edge = new HyperEdge(b_node);
        b_edge->AddTail(c_node); b_edge->AddTail(d_node);
        b_node->AddEdge(b_edge); tree_exp.AddEdge(b_edge);
        // Make the edge for C
        HyperEdge * c_edge = new HyperEdge(c_node);
        c_edge->AddTail(x_node);
        c_node->AddEdge(c_edge); tree_exp.AddEdge(c_edge);
        // Make the edge for D
        HyperEdge * d_edge = new HyperEdge(d_node);
        d_edge->AddTail(y_node);
        d_node->AddEdge(d_edge); tree_exp.AddEdge(d_edge);
        // Make the edge for E
        HyperEdge * e_edge = new HyperEdge(e_node);
        e_edge->AddTail(z_node);
        e_node->AddEdge(e_edge); tree_exp.AddEdge(e_edge);

        graph_str = "{\"words\": [\"running\", \"water\"], \"nodes\": [ {\"id\": 0, \"sym\": \"ROOT\", \"span\": [0, 2], \"trg_span\": [0, 2]}, {\"id\": 1, \"sym\": \"VP\", \"span\": [0, 2]}, {\"id\": 2, \"sym\": \"NP\", \"span\": [0, 2]}, {\"id\": 3, \"sym\": \"JJ\", \"span\": [0, 1]}, {\"id\": 4, \"sym\": \"VP\", \"span\": [0, 1]}, {\"id\": 5, \"sym\": \"VPG\", \"span\": [0, 1]}, {\"id\": 6, \"sym\": \"running\", \"span\": [0, 1]}, {\"id\": 7, \"sym\": \"NP\", \"span\": [1, 2]}, {\"id\": 8, \"sym\": \"NN\", \"span\": [1, 2]}, {\"id\": 9, \"sym\": \"water\", \"span\": [1, 2]} ], \"edges\": [ {\"id\": 0, \"head\": 0, \"tails\": [1], \"score\": 1.0, \"features\": {\"parse\": 1.0}}, {\"id\": 1, \"head\": 0, \"tails\": [2]}, {\"id\": 2, \"head\": 1, \"tails\": [4, 7]}, {\"id\": 3, \"head\": 2, \"tails\": [3, 8]}, {\"id\": 4, \"head\": 3, \"tails\": [6]}, {\"id\": 5, \"head\": 4, \"tails\": [5]}, {\"id\": 6, \"head\": 5, \"tails\": [6]}, {\"id\": 7, \"head\": 7, \"tails\": [8]}, {\"id\": 8, \"head\": 8, \"tails\": [9]}, {\"id\": 9, \"head\": 8, \"trg\": [\"\\\\\\\"\", -1]} ]}\nAAA";

        // Create the words
        {
        HyperNode* node0 = new HyperNode(Dict::WID("ROOT"), -1,    make_pair(0,2)); graph_exp.AddNode(node0);
        node0->GetTrgSpan().insert(0); node0->GetTrgSpan().insert(2);
        HyperNode* node1 = new HyperNode(Dict::WID("VP"), -1,      make_pair(0,2)); graph_exp.AddNode(node1);
        HyperNode* node2 = new HyperNode(Dict::WID("NP"), -1,      make_pair(0,2)); graph_exp.AddNode(node2);
        HyperNode* node3 = new HyperNode(Dict::WID("JJ"), -1,      make_pair(0,1)); graph_exp.AddNode(node3);
        HyperNode* node4 = new HyperNode(Dict::WID("VP"), -1,      make_pair(0,1)); graph_exp.AddNode(node4);
        HyperNode* node5 = new HyperNode(Dict::WID("VPG"), -1,     make_pair(0,1)); graph_exp.AddNode(node5);
        HyperNode* node6 = new HyperNode(Dict::WID("running"), -1, make_pair(0,1)); graph_exp.AddNode(node6);
        HyperNode* node7 = new HyperNode(Dict::WID("NP"), -1,      make_pair(1,2)); graph_exp.AddNode(node7);
        HyperNode* node8 = new HyperNode(Dict::WID("NN"), -1,      make_pair(1,2)); graph_exp.AddNode(node8);
        HyperNode* node9 = new HyperNode(Dict::WID("water"), -1,   make_pair(1,2)); graph_exp.AddNode(node9);
        HyperEdge* edge0 = new HyperEdge(node0); edge0->AddTail(node1); node0->AddEdge(edge0); graph_exp.AddEdge(edge0); edge0->SetScore(1.0); edge0->AddFeature(Dict::WID("parse"), 1.0);
        HyperEdge* edge1 = new HyperEdge(node0); edge1->AddTail(node2); node0->AddEdge(edge1); graph_exp.AddEdge(edge1);
        HyperEdge* edge2 = new HyperEdge(node1); edge2->AddTail(node4); edge2->AddTail(node7); node1->AddEdge(edge2); graph_exp.AddEdge(edge2);
        HyperEdge* edge3 = new HyperEdge(node2); edge3->AddTail(node3); edge3->AddTail(node8); node2->AddEdge(edge3); graph_exp.AddEdge(edge3);
        HyperEdge* edge4 = new HyperEdge(node3); edge4->AddTail(node6); node3->AddEdge(edge4); graph_exp.AddEdge(edge4);
        HyperEdge* edge5 = new HyperEdge(node4); edge5->AddTail(node5); node4->AddEdge(edge5); graph_exp.AddEdge(edge5);
        HyperEdge* edge6 = new HyperEdge(node5); edge6->AddTail(node6); node5->AddEdge(edge6); graph_exp.AddEdge(edge6);
        HyperEdge* edge7 = new HyperEdge(node7); edge7->AddTail(node8); node7->AddEdge(edge7); graph_exp.AddEdge(edge7);
        HyperEdge* edge8 = new HyperEdge(node8); edge8->AddTail(node9); node8->AddEdge(edge8); graph_exp.AddEdge(edge8);
        HyperEdge* edge9 = new HyperEdge(node8); node8->AddEdge(edge9); graph_exp.AddEdge(edge9);
        edge9->GetTrgWords().push_back(Dict::WID("\\\""));
        edge9->GetTrgWords().push_back(-1);
        graph_exp.SetWords(Dict::ParseWords("running water"));
        }
 
        HyperNode* quotenode = new HyperNode(Dict::WID("\""), -1,    make_pair(0,1)); quote_exp.AddNode(quotenode);
        quote_exp.SetWords(Dict::ParseWords("\""));

        egret_str=
"sentence 1 :\n"
"story of man \n"
"NN[0,0] => story ||| -5.16e-06\n"
"NP^g[0,0] => NN[0,0]  ||| -0.17\n"
"IN[1,1] => of ||| -3.32e-06\n"
"NN[2,2] => man ||| -0.02\n"
"NP^g[2,2] => NN[2,2]  ||| -0.02\n"
"PP^g[1,2] => IN[1,1] NP^g[2,2]  ||| -1.40e-05\n"
"ADJP^g[0,2] => NN[0,0] PP^g[1,2]  ||| -7.66\n"
"NP^g[0,2] => NP^g[0,0] PP^g[1,2]  ||| -0.19\n"
"VP^g[0,2] => NN[0,0] PP^g[1,2]  ||| -3.19\n"
"S^g[0,2] => ADJP^g[0,2]  ||| -27.43\n"
"ROOT[0,2] => NP^g[0,2]  ||| -0.19\n"
"ROOT[0,2] => S^g[0,2]  ||| -3.19\n"
"ROOT[0,2] => VP^g[0,2]  ||| -3.19\n"
"\n"
"sentence 2 :";

        HyperNode* nn00    = new HyperNode(Dict::WID("NN"), -1,      make_pair(0,1)); egret_exp.AddNode(nn00   );
        HyperNode* story00 = new HyperNode(Dict::WID("story"), -1,   make_pair(0,1)); egret_exp.AddNode(story00);
        HyperNode* npg00   = new HyperNode(Dict::WID("NP^g"), -1,    make_pair(0,1)); egret_exp.AddNode(npg00  );
        HyperNode* in11    = new HyperNode(Dict::WID("IN"), -1,      make_pair(1,2)); egret_exp.AddNode(in11   );
        HyperNode* of11    = new HyperNode(Dict::WID("of"), -1,      make_pair(1,2)); egret_exp.AddNode(of11   );
        HyperNode* nn22    = new HyperNode(Dict::WID("NN"), -1,      make_pair(2,3)); egret_exp.AddNode(nn22   );
        HyperNode* man22   = new HyperNode(Dict::WID("man"), -1,     make_pair(2,3)); egret_exp.AddNode(man22  );
        HyperNode* npg22   = new HyperNode(Dict::WID("NP^g"), -1,    make_pair(2,3)); egret_exp.AddNode(npg22  );
        HyperNode* ppg12   = new HyperNode(Dict::WID("PP^g"), -1,    make_pair(1,3)); egret_exp.AddNode(ppg12  );
        HyperNode* adjpg02 = new HyperNode(Dict::WID("ADJP^g"), -1,  make_pair(0,3)); egret_exp.AddNode(adjpg02);
        HyperNode* npg02   = new HyperNode(Dict::WID("NP^g"), -1,    make_pair(0,3)); egret_exp.AddNode(npg02  );
        HyperNode* vpg02   = new HyperNode(Dict::WID("VP^g"), -1,    make_pair(0,3)); egret_exp.AddNode(vpg02  );
        HyperNode* sg02    = new HyperNode(Dict::WID("S^g"), -1,     make_pair(0,3)); egret_exp.AddNode(sg02   );
        HyperNode* root02  = new HyperNode(Dict::WID("ROOT"), -1,    make_pair(0,3)); egret_exp.AddNode(root02 );
        nn00->SetId(13); egret_exp.GetNodes()[13] = nn00;
        root02->SetId(0); egret_exp.GetNodes()[0] = root02;
        HyperEdge* edge0   = new HyperEdge(nn00);   nn00->AddEdge(edge0); edge0->AddTail(story00);  egret_exp.AddEdge(edge0);  edge0->SetScore(-5.16e-06); edge0->AddFeature(Dict::WID("parse"), -5.16e-06);
        HyperEdge* edge1   = new HyperEdge(npg00);  npg00->AddEdge(edge1); edge1->AddTail(nn00);   egret_exp.AddEdge(edge1);  edge1->SetScore(-0.17); edge1->AddFeature(Dict::WID("parse"), -0.17);
        HyperEdge* edge2   = new HyperEdge(in11);   in11->AddEdge(edge2); edge2->AddTail(of11);   egret_exp.AddEdge(edge2);  edge2->SetScore(-3.32e-06); edge2->AddFeature(Dict::WID("parse"), -3.32e-06);
        HyperEdge* edge3   = new HyperEdge(nn22);   nn22->AddEdge(edge3); edge3->AddTail(man22);  egret_exp.AddEdge(edge3);  edge3->SetScore(-0.02); edge3->AddFeature(Dict::WID("parse"), -0.02);
        HyperEdge* edge4   = new HyperEdge(npg22);  npg22->AddEdge(edge4); edge4->AddTail(nn22);   egret_exp.AddEdge(edge4);  edge4->SetScore(-0.02); edge4->AddFeature(Dict::WID("parse"), -0.02);
        HyperEdge* edge5   = new HyperEdge(ppg12);  ppg12->AddEdge(edge5); edge5->AddTail(in11);   edge5->AddTail(npg22);  egret_exp.AddEdge(edge5);  edge5->SetScore(-1.40e-05); edge5->AddFeature(Dict::WID("parse"), -1.40e-05);
        HyperEdge* edge6   = new HyperEdge(adjpg02);adjpg02->AddEdge(edge6); edge6->AddTail(nn00);   edge6->AddTail(ppg12);  egret_exp.AddEdge(edge6);  edge6->SetScore(-7.66); edge6->AddFeature(Dict::WID("parse"), -7.66);
        HyperEdge* edge7   = new HyperEdge(npg02);  npg02->AddEdge(edge7); edge7->AddTail(npg00);  edge7->AddTail(ppg12);  egret_exp.AddEdge(edge7);  edge7->SetScore(-0.19); edge7->AddFeature(Dict::WID("parse"), -0.19);
        HyperEdge* edge8   = new HyperEdge(vpg02);  vpg02->AddEdge(edge8); edge8->AddTail(nn00);   edge8->AddTail(ppg12);  egret_exp.AddEdge(edge8);  edge8->SetScore(-3.19); edge8->AddFeature(Dict::WID("parse"), -3.19);
        HyperEdge* edge9   = new HyperEdge(sg02);   sg02->AddEdge(edge9); edge9->AddTail(adjpg02);  egret_exp.AddEdge(edge9);  edge9->SetScore(-27.43); edge9->AddFeature(Dict::WID("parse"), -27.43);
        HyperEdge* edge10  = new HyperEdge(root02); root02->AddEdge(edge10); edge10->AddTail(npg02); egret_exp.AddEdge(edge10); edge10->SetScore(-0.19); edge10->AddFeature(Dict::WID("parse"), -0.19);
        HyperEdge* edge11  = new HyperEdge(root02); root02->AddEdge(edge11); edge11->AddTail(sg02); egret_exp.AddEdge(edge11); edge11->SetScore(-3.19); edge11->AddFeature(Dict::WID("parse"), -3.19);
        HyperEdge* edge12  = new HyperEdge(root02); root02->AddEdge(edge12); edge12->AddTail(vpg02); egret_exp.AddEdge(edge12); edge12->SetScore(-3.19); edge12->AddFeature(Dict::WID("parse"), -3.19);
        egret_exp.SetWords(Dict::ParseWords("story of man"));
 
    }
    ~TestTreeIO() { }

    int TestReadPenn() {
        // Use this tree_str
        istringstream instr(tree_str);
        PennTreeIO io;
        boost::scoped_ptr<HyperGraph> hg_act(io.ReadTree(instr));
        // Check to make sure that the remaining values still remain
        string left_act; instr >> left_act;
        string left_exp = "AAA";
        // Check that both values are equal
        return tree_exp.CheckEqual(*hg_act) && left_act == left_exp;
    }

    int TestReadEgret() {
        // Use this tree
        istringstream instr(egret_str);
        EgretTreeIO io;
        boost::scoped_ptr<HyperGraph> hg_act(io.ReadTree(instr));
        // Check that both values are equal
        return egret_exp.CheckEqual(*hg_act);
    }

    int TestReadJSON() {
        // Use this tree
        istringstream instr(graph_str);
        JSONTreeIO io;
        boost::scoped_ptr<HyperGraph> hg_act(io.ReadTree(instr));
        // Check to make sure that the remaining values still remain
        string left_act; instr >> left_act;
        string left_exp = "AAA";
        // Check that both values are equal
        return graph_exp.CheckEqual(*hg_act) && left_act == left_exp;
    }

    int TestWriteJSON() {
        // Use this tree
        stringstream strm;
        JSONTreeIO io;
        io.WriteTree(graph_exp, strm);
        boost::scoped_ptr<HyperGraph> hg_act(io.ReadTree(strm));
        // Check that both values are equal
        return graph_exp.CheckEqual(*hg_act);
    }

    int TestWriteJSONQuote() {
        // Use this tree
        stringstream strm;
        JSONTreeIO io;
        io.WriteTree(quote_exp, strm);
        boost::scoped_ptr<HyperGraph> hg_act(io.ReadTree(strm));
        // Check that both values are equal
        return quote_exp.CheckEqual(*hg_act);
    }

    int TestWritePenn() {
        string tree_str = "(A (B (C x) (D y)) (E z))";
        PennTreeIO penn;
        istringstream iss(tree_str);
        shared_ptr<HyperGraph> graph(penn.ReadTree(iss));
        ostringstream oss; penn.WriteTree(*graph, oss);
        string act_str = oss.str();
        return CheckEqual(tree_str, act_str);
    }

    int TestWriteMosesXML() {
        string tree_str = "(A (B (C x) (D y)) (E z))";
        string exp_str = "<tree label=\"A\"> <tree label=\"B\"> <tree label=\"C\"> x </tree> <tree label=\"D\"> y </tree> </tree> <tree label=\"E\"> z </tree> </tree>";
        PennTreeIO penn;
        MosesXMLTreeIO moses;
        istringstream iss(tree_str);
        shared_ptr<HyperGraph> graph(penn.ReadTree(iss));
        ostringstream oss; moses.WriteTree(*graph, oss);
        string act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestReadJSON()" << endl; if(TestReadJSON()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWriteJSON()" << endl; if(TestWriteJSON()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWriteJSONQuote()" << endl; if(TestWriteJSONQuote()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWritePenn()" << endl; if(TestWritePenn()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWriteMosesXML()" << endl; if(TestWriteMosesXML()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestReadPenn()" << endl; if(TestReadPenn()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestReadEgret()" << endl; if(TestReadEgret()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTreeIO Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

    string tree_str, graph_str, egret_str;
    HyperGraph tree_exp, graph_exp, quote_exp, egret_exp;

};

}

#endif
