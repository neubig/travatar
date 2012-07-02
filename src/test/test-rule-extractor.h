#ifndef TEST_RULE_EXTRACTOR_H__
#define TEST_RULE_EXTRACTOR_H__

#include "test-base.h"
#include <travatar/rule-extractor.h>
#include <travatar/alignment.h>
#include <boost/shared_ptr.hpp>

using namespace boost;

namespace travatar {

class TestRuleExtractor : public TestBase {

public:

    TestRuleExtractor() {
        // Use the example from Galley et al.
        string src1_tree = "(ROOT (S (NP (PRP he)) (VP (AUX does) (RB not) (VB go))))";
        string trg1_str  = "il ne va pas";
        string align1_str = "0-0 1-1 1-3 2-1 2-3 3-2";
        istringstream iss1(src1_tree);
        src1_graph.reset(tree_io.ReadTree(iss1));
        trg1_sent = Dict::ParseWords(trg1_str);
        align1 = Alignment::FromString(align1_str);
        // also an example of a forest
        src2_graph.reset(new HyperGraph);
        HyperNode* node0 = new HyperNode(Dict::WID("ROOT"),    MakePair(0,2)); src2_graph->AddNode(node0);
        HyperNode* node1 = new HyperNode(Dict::WID("VP"),      MakePair(0,2)); src2_graph->AddNode(node1);
        HyperNode* node2 = new HyperNode(Dict::WID("NP"),      MakePair(0,2)); src2_graph->AddNode(node2);
        HyperNode* node3 = new HyperNode(Dict::WID("JJ"),      MakePair(0,1)); src2_graph->AddNode(node3);
        HyperNode* node4 = new HyperNode(Dict::WID("VP"),      MakePair(0,1)); src2_graph->AddNode(node4);
        HyperNode* node5 = new HyperNode(Dict::WID("VPG"),     MakePair(0,1)); src2_graph->AddNode(node5);
        HyperNode* node6 = new HyperNode(Dict::WID("running"), MakePair(0,1)); src2_graph->AddNode(node6);
        HyperNode* node7 = new HyperNode(Dict::WID("NP"),      MakePair(1,2)); src2_graph->AddNode(node7);
        HyperNode* node8 = new HyperNode(Dict::WID("NN"),      MakePair(1,2)); src2_graph->AddNode(node8);
        HyperNode* node9 = new HyperNode(Dict::WID("water"),   MakePair(1,2)); src2_graph->AddNode(node9);
        HyperEdge* edge0 = new HyperEdge(node0); edge0->AddTail(node1); node0->AddEdge(edge0); src2_graph->AddEdge(edge0);
        HyperEdge* edge1 = new HyperEdge(node0); edge1->AddTail(node2); node0->AddEdge(edge1); src2_graph->AddEdge(edge1);
        HyperEdge* edge2 = new HyperEdge(node1); edge2->AddTail(node4); edge2->AddTail(node7); node1->AddEdge(edge2); src2_graph->AddEdge(edge2);
        HyperEdge* edge3 = new HyperEdge(node2); edge3->AddTail(node3); edge3->AddTail(node8); node2->AddEdge(edge3); src2_graph->AddEdge(edge3);
        HyperEdge* edge4 = new HyperEdge(node3); edge4->AddTail(node6); node3->AddEdge(edge4); src2_graph->AddEdge(edge4);
        HyperEdge* edge5 = new HyperEdge(node4); edge5->AddTail(node5); node4->AddEdge(edge5); src2_graph->AddEdge(edge5);
        HyperEdge* edge6 = new HyperEdge(node5); edge6->AddTail(node6); node5->AddEdge(edge6); src2_graph->AddEdge(edge6);
        HyperEdge* edge7 = new HyperEdge(node7); edge7->AddTail(node8); node7->AddEdge(edge7); src2_graph->AddEdge(edge7);
        HyperEdge* edge8 = new HyperEdge(node8); edge8->AddTail(node9); node8->AddEdge(edge8); src2_graph->AddEdge(edge8);
        src2_graph->SetWords(Dict::ParseWords("running water"));
        string trg2_str  = "mizu no nagare"; // Handle this translation with caution :)
        string align2_str = "0-2 1-0";
        trg2_sent = Dict::ParseWords(trg2_str);
        align2 = Alignment::FromString(align2_str);
        // An example of a tree with source and target side null words
        string src3_tree = "(ROOT (VP (VBD ate) (NP (NN rice))))";
        string trg3_str  = "gohan o tabeta";
        string align3_str = "0-2 1-0";
        istringstream iss3(src3_tree);
        src3_graph.reset(tree_io.ReadTree(iss3));
        trg3_sent = Dict::ParseWords(trg3_str);
        align3 = Alignment::FromString(align3_str);
    }

    ~TestRuleExtractor() { }

    int TestTreeExtraction() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        scoped_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src1_graph, align1));
        // Create the actual rule graph
        HyperGraph frags_exp;
        frags_exp.SetWords(src1_graph->GetWords());
        // ---- Add nodes ----
        // Expected node numbers: "(ROOT0 (S1 (NP2 (PRP3 he)) (VP4 (AUX5 does) (RB6 not) (VB7 go))))";
        // Node rooted at root0
        HyperNode* root0_node = new HyperNode(Dict::WID("ROOT"), MakePair(0,4));
        root0_node->SetTrgSpan(src1_graph->GetNode(0)->GetTrgSpan());
        frags_exp.AddNode(root0_node);
        // Node rooted at s1
        HyperNode* s1_node = new HyperNode(Dict::WID("S"), MakePair(0,4));
        s1_node->SetTrgSpan(src1_graph->GetNode(1)->GetTrgSpan());
        frags_exp.AddNode(s1_node);
        // Node rooted at np2
        HyperNode* np2_node = new HyperNode(Dict::WID("NP"), MakePair(0,1));
        np2_node->SetTrgSpan(src1_graph->GetNode(2)->GetTrgSpan());
        frags_exp.AddNode(np2_node);
        // Node rooted at prp3
        HyperNode* prp3_node = new HyperNode(Dict::WID("PRP"), MakePair(0,1));
        prp3_node->SetTrgSpan(src1_graph->GetNode(3)->GetTrgSpan());
        frags_exp.AddNode(prp3_node);
        // Node rooted at vp4
        HyperNode* vp4_node = new HyperNode(Dict::WID("VP"), MakePair(1,4));
        vp4_node->SetTrgSpan(src1_graph->GetNode(5)->GetTrgSpan());
        frags_exp.AddNode(vp4_node);
        // Node rooted at vb7
        HyperNode* vb7_node = new HyperNode(Dict::WID("VB"), MakePair(3,4));
        vb7_node->SetTrgSpan(src1_graph->GetNode(10)->GetTrgSpan());
        frags_exp.AddNode(vb7_node);
        // ---- Add edges ----
        // Edge for root0
        HyperEdge* root0_edge = new HyperEdge(root0_node);
        frags_exp.AddEdge(root0_edge);
        root0_edge->AddTail(s1_node);
        root0_edge->AddFragmentEdge(src1_graph->GetEdge(0));
        root0_node->AddEdge(root0_edge);
        // Edge for s1
        HyperEdge* s1_edge = new HyperEdge(s1_node);
        frags_exp.AddEdge(s1_edge);
        s1_edge->AddTail(np2_node);
        s1_edge->AddTail(vp4_node);
        s1_edge->AddFragmentEdge(src1_graph->GetEdge(1));
        s1_node->AddEdge(s1_edge);
        // Edge for np2
        HyperEdge* np2_edge = new HyperEdge(np2_node);
        frags_exp.AddEdge(np2_edge);
        np2_edge->AddTail(prp3_node);
        np2_edge->AddFragmentEdge(src1_graph->GetEdge(2));
        np2_node->AddEdge(np2_edge);
        // Edge for prp3
        HyperEdge* prp3_edge = new HyperEdge(prp3_node);
        frags_exp.AddEdge(prp3_edge);
        prp3_edge->AddFragmentEdge(src1_graph->GetEdge(3));
        prp3_node->AddEdge(prp3_edge);
        // Edge for vp4
        HyperEdge* vp4_edge = new HyperEdge(vp4_node);
        frags_exp.AddEdge(vp4_edge);
        vp4_edge->AddTail(vb7_node);
        vp4_edge->AddFragmentEdge(src1_graph->GetEdge(4));
        vp4_edge->AddFragmentEdge(src1_graph->GetEdge(5));
        vp4_edge->AddFragmentEdge(src1_graph->GetEdge(6));
        vp4_node->AddEdge(vp4_edge);
        // Edge for vb7
        HyperEdge* vb7_edge = new HyperEdge(vb7_node);
        frags_exp.AddEdge(vb7_edge);
        vb7_edge->AddFragmentEdge(src1_graph->GetEdge(7));
        vb7_node->AddEdge(vb7_edge);
        // Check to make sure that these are equal
        return frags_exp.CheckEqual(*frags_act);
    }

    int TestForestExtraction() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        src2_graph->NormalizeEdgeProbabilities();
        shared_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src2_graph, align2));
        // Create the actual values, all non-terminal edges should be fine, and most should have
        // a probability of 0.5, as they only belong to one of the two trees
        vector<double> scores_exp(9, 1.0), scores_act(9);
        for(int i = 0; i < 9; i++) 
            scores_act[i] = frags_act->GetEdge(i)->GetProb();
        // The only two edges that are 0.5 are the split edges on top
        scores_exp[0] = 0.5; scores_exp[1] = 0.5;
        // Check to make sure that these are equal
        return CheckVector(scores_exp, scores_act);
    }

    int TestTopNullExtraction() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        scoped_ptr<HyperGraph> frags_min(forest_ext.ExtractMinimalRules(*src3_graph, align3));
        scoped_ptr<HyperGraph> frags_act(forest_ext.AttachNullsTop(*frags_min, align3, trg3_sent.size()));
        // Create the actual rule graph
        // Expected nodes "(ROOT0 (VP1 (VBD2 ate) (NP4 (NN5 rice))))";
        HyperGraph frags_exp;
        frags_exp.SetWords(src3_graph->GetWords());
        // ---- Add nodes ----
        // Node rooted at root0
        HyperNode* root0_node = new HyperNode(Dict::WID("ROOT"), MakePair(0,2));
        root0_node->SetTrgSpan(src3_graph->GetNode(0)->GetTrgSpan());
        frags_exp.AddNode(root0_node);
        // Node rooted at vp1
        HyperNode* vp1_node = new HyperNode(Dict::WID("VP"), MakePair(0,2));
        vp1_node->SetTrgSpan(src3_graph->GetNode(1)->GetTrgSpan());
        vp1_node->GetTrgSpan().insert(1);
        frags_exp.AddNode(vp1_node);
        // Node rooted at vbd2 without "o"
        HyperNode* vbd2_node = new HyperNode(Dict::WID("VBD"), MakePair(0,1));
        vbd2_node->SetTrgSpan(src3_graph->GetNode(2)->GetTrgSpan());
        frags_exp.AddNode(vbd2_node);
        // Node rooted at np4 without "o"
        HyperNode* np4_node = new HyperNode(Dict::WID("NP"), MakePair(1,2));
        np4_node->SetTrgSpan(src3_graph->GetNode(4)->GetTrgSpan());
        frags_exp.AddNode(np4_node);
        // Node rooted at nn5 without "o"
        HyperNode* nn5_node = new HyperNode(Dict::WID("NN"), MakePair(1,2));
        nn5_node->SetTrgSpan(src3_graph->GetNode(5)->GetTrgSpan());
        frags_exp.AddNode(nn5_node);
        // ---- Add edges ----
        // Edge for root0
        HyperEdge* root0_edge = new HyperEdge(root0_node);
        frags_exp.AddEdge(root0_edge);
        root0_edge->AddTail(vp1_node);
        root0_edge->AddFragmentEdge(src1_graph->GetEdge(0));
        root0_node->AddEdge(root0_edge);
        // Edge for vp1
        HyperEdge* vp1_edge = new HyperEdge(vp1_node);
        frags_exp.AddEdge(vp1_edge);
        vp1_edge->AddTail(vbd2_node);
        vp1_edge->AddTail(np4_node);
        vp1_edge->AddFragmentEdge(src1_graph->GetEdge(1));
        vp1_node->AddEdge(vp1_edge);
        // Edge for vbd2
        HyperEdge* vbd2_edge = new HyperEdge(vbd2_node);
        frags_exp.AddEdge(vbd2_edge);
        vbd2_edge->AddFragmentEdge(src1_graph->GetEdge(2));
        vbd2_node->AddEdge(vbd2_edge);
        // Edge for np4
        HyperEdge* np4_edge = new HyperEdge(np4_node);
        frags_exp.AddEdge(np4_edge);
        np4_edge->AddTail(nn5_node);
        np4_edge->AddFragmentEdge(src1_graph->GetEdge(3));
        np4_node->AddEdge(np4_edge);
        // Edge for nn5
        HyperEdge* nn5_edge = new HyperEdge(nn5_node);
        frags_exp.AddEdge(nn5_edge);
        nn5_edge->AddFragmentEdge(src1_graph->GetEdge(4));
        nn5_node->AddEdge(nn5_edge);
        return frags_exp.CheckEqual(*frags_act);
    }

    int TestExhaustiveNullExtraction() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        scoped_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src3_graph, align3));
        // Create the actual rule graph
        // Expected nodes "(ROOT0 (VP1 (VBD2 ate) (NP4 (NN5 rice))))";
        HyperGraph frags_exp;
        frags_exp.SetWords(src3_graph->GetWords());
        // ---- Add nodes ----
        // Node rooted at root0
        HyperNode* root0_node = new HyperNode(Dict::WID("ROOT"), MakePair(0,2));
        root0_node->SetTrgSpan(src3_graph->GetNode(0)->GetTrgSpan());
        frags_exp.AddNode(root0_node);
        // Node rooted at vp1 without "o"
        HyperNode* vp1_node_n = new HyperNode(Dict::WID("VP"), MakePair(0,2));
        vp1_node_n->SetTrgSpan(src3_graph->GetNode(1)->GetTrgSpan());
        frags_exp.AddNode(vp1_node_n);
        // Node rooted at vp1 with "0"
        HyperNode* vp1_node_y = new HyperNode(Dict::WID("VP"), MakePair(0,2));
        vp1_node_y->SetTrgSpan(src3_graph->GetNode(1)->GetTrgSpan());
        vp1_node_y->GetTrgSpan().insert(1);
        frags_exp.AddNode(vp1_node_y);
        // Node rooted at vbd2 without "o"
        HyperNode* vbd2_node_n = new HyperNode(Dict::WID("VBD"), MakePair(0,1));
        vbd2_node_n->SetTrgSpan(src3_graph->GetNode(2)->GetTrgSpan());
        frags_exp.AddNode(vbd2_node_n);
        // Node rooted at vbd2 with "o"
        HyperNode* vbd2_node_y = new HyperNode(Dict::WID("VBD"), MakePair(0,1));
        vbd2_node_y->SetTrgSpan(src3_graph->GetNode(2)->GetTrgSpan());
        vbd2_node_y->GetTrgSpan().insert(1);
        frags_exp.AddNode(vbd2_node_y);
        // Node rooted at np4 without "o"
        HyperNode* np4_node_n = new HyperNode(Dict::WID("NP"), MakePair(1,2));
        np4_node_n->SetTrgSpan(src3_graph->GetNode(4)->GetTrgSpan());
        frags_exp.AddNode(np4_node_n);
        // Node rooted at np4 with "o"
        HyperNode* np4_node_y = new HyperNode(Dict::WID("NP"), MakePair(1,2));
        np4_node_y->SetTrgSpan(src3_graph->GetNode(4)->GetTrgSpan());
        np4_node_y->GetTrgSpan().insert(1);
        frags_exp.AddNode(np4_node_y);
        // Node rooted at nn5 without "o"
        HyperNode* nn5_node_n = new HyperNode(Dict::WID("NN"), MakePair(1,2));
        nn5_node_n->SetTrgSpan(src3_graph->GetNode(5)->GetTrgSpan());
        frags_exp.AddNode(nn5_node_n);
        // Node rooted at nn5 with "o"
        HyperNode* nn5_node_y = new HyperNode(Dict::WID("NN"), MakePair(1,2));
        nn5_node_y->SetTrgSpan(src3_graph->GetNode(5)->GetTrgSpan());
        nn5_node_y->GetTrgSpan().insert(1);
        frags_exp.AddNode(nn5_node_y);
        // ---- Add edges ----
        // Edge for root0
        HyperEdge* root0_edge_y = new HyperEdge(root0_node);
        frags_exp.AddEdge(root0_edge_y);
        root0_edge_y->AddTail(vp1_node_y);
        root0_edge_y->AddFragmentEdge(src1_graph->GetEdge(0));
        root0_node->AddEdge(root0_edge_y);
        // Edge for root0
        HyperEdge* root0_edge_n = new HyperEdge(root0_node);
        frags_exp.AddEdge(root0_edge_n);
        root0_edge_n->AddTail(vp1_node_n);
        root0_edge_n->AddFragmentEdge(src1_graph->GetEdge(0));
        root0_node->AddEdge(root0_edge_n);
        return frags_exp.CheckEqual(*frags_act);
    }

    int TestRulePrinting() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        shared_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src1_graph, align1));
        vector<string> rule_exp, rule_act;
        BOOST_FOREACH(HyperEdge* edge, frags_act->GetEdges())
            rule_act.push_back(forest_ext.RuleToString(*edge, src1_graph->GetWords(), trg1_sent));
        rule_exp.push_back("ROOT ( x0:S ) ||| x0 ||| 1");
        rule_exp.push_back("S ( x0:NP x1:VP ) ||| x0 x1 ||| 1");
        rule_exp.push_back("NP ( x0:PRP ) ||| x0 ||| 1");
        rule_exp.push_back("PRP ( \"he\" ) ||| \"il\" ||| 1");
        rule_exp.push_back("VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB ) ||| \"ne\" x0 \"pas\" ||| 1");
        rule_exp.push_back("VB ( \"go\" ) ||| \"va\" ||| 1");
        return CheckVector(rule_exp, rule_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestTreeExtraction()" << endl; if(TestTreeExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestForestExtraction()" << endl; if(TestForestExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestTopNullExtraction()" << endl; if(TestTopNullExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        // done++; cout << "TestExhaustiveNullExtraction()" << endl; if(TestExhaustiveNullExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRulePrinting()" << endl; if(TestRulePrinting()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestRuleExtractor Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    PennTreeIO tree_io;
    boost::scoped_ptr<HyperGraph> src1_graph, src2_graph, src3_graph;
    Sentence trg1_sent, trg2_sent, trg3_sent;
    Alignment align1, align2, align3;

};

}

#endif