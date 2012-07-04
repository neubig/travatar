#ifndef TEST_HYPER_GRAPH_H__
#define TEST_HYPER_GRAPH_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <boost/shared_ptr.hpp>

using namespace boost;

namespace travatar {

class TestHyperGraph : public TestBase {

public:

    TestHyperGraph() {
        // Use the example from Galley et al.
        string src1_tree = "(S (NP (PRP he)) (VP (AUX does) (RB not) (VB go)))";
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
        // Use an example with a target null
        string src3_tree = "(S (NP (PRP he)) (VP (VB went)))";
        string trg3_str  = "ikimashita";
        string align3_str = "1-0";
        istringstream iss3(src3_tree);
        src3_graph.reset(tree_io.ReadTree(iss3));
        trg3_sent = Dict::ParseWords(trg3_str);
        align3 = Alignment::FromString(align3_str);
    }


    ~TestHyperGraph() { }

    int TestCalculateSpan() {
        vector<set<int> > src_span = align1.GetSrcAlignments();
        src1_graph->GetNode(0)->CalculateTrgSpan(src_span);
        // Save the actual spans
        vector<set<int> > node_exp(11);
        node_exp[0].insert(0); node_exp[0].insert(1); node_exp[0].insert(2); node_exp[0].insert(3);
        node_exp[1].insert(0);
        node_exp[2].insert(0);
        node_exp[3].insert(0);
        node_exp[4].insert(1); node_exp[4].insert(2); node_exp[4].insert(3);
        node_exp[5].insert(1); node_exp[5].insert(3);
        node_exp[6].insert(1); node_exp[6].insert(3);
        node_exp[7].insert(1); node_exp[7].insert(3);
        node_exp[8].insert(1); node_exp[8].insert(3);
        node_exp[9].insert(2);
        node_exp[10].insert(2);
        // Check the equality
        int ret = 1;
        for(int i = 0; i < 11; i++)
            if(node_exp[i] != src1_graph->GetNode(i)->GetTrgSpan()) {
                cerr << i << " not equal" << endl;
                ret = 0;
            }
        return ret;
    }

    int TestCopy() {
        HyperGraph src1_copy(*src1_graph);
        return src1_graph->CheckEqual(src1_copy);
    }

    int TestCalculateSpanForest() {
        vector<set<int> > src_span = align2.GetSrcAlignments();
        vector<set<int>*> node_span(src2_graph->NumNodes(), (set<int>*)NULL);
        for(int i = 0; i < 10; i++)
            src2_graph->GetNode(i)->CalculateTrgSpan(src_span);
        // Save the actual spans
        vector<set<int> > node_exp(10);
        node_exp[0].insert(0); node_exp[0].insert(2);
        node_exp[1].insert(0); node_exp[1].insert(2);
        node_exp[2].insert(0); node_exp[2].insert(2);
        node_exp[3].insert(2);
        node_exp[4].insert(2);
        node_exp[5].insert(2); 
        node_exp[6].insert(2); 
        node_exp[7].insert(0); 
        node_exp[8].insert(0); 
        node_exp[9].insert(0);
        // Check the equality
        int ret = 1;
        for(int i = 0; i < 10; i++)
            if(node_exp[i] != src2_graph->GetNode(i)->GetTrgSpan()) {
                ret = 0;
                cerr << i << " not equal " << SafeReference(SafeAccess(node_span, i)) << endl;
            }
        return ret;
    }

    int TestCalculateFrontier() {
        vector<set<int> > src_span = align1.GetSrcAlignments();
        vector<set<int>*> node_span(src1_graph->NumNodes(), (set<int>*)NULL);
        // Here, we will treat terminal nodes as non-frontier, as we just
        vector<HyperNode::FrontierType> 
            exp(11, HyperNode::IS_FRONTIER), act(11, HyperNode::IS_FRONTIER);
        exp[3] = HyperNode::NOT_FRONTIER;
        exp[5] = HyperNode::NOT_FRONTIER; exp[6] = HyperNode::NOT_FRONTIER;
        exp[7] = HyperNode::NOT_FRONTIER; exp[8] = HyperNode::NOT_FRONTIER;
        exp[10] = HyperNode::NOT_FRONTIER;
        src1_graph->CalculateFrontiers(src_span);
        for(int i = 0; i < 11; i++)
            act[i] = src1_graph->GetNode(i)->GetFrontier();
        return CheckVector(exp, act);
    }
    
    int TestCalculateNull() {
        vector<set<int> > src_span = align3.GetSrcAlignments();
        vector<set<int>*> node_span(src3_graph->NumNodes(), (set<int>*)NULL);
        // Here, we will treat terminal nodes as non-frontier, as we just
        vector<HyperNode::FrontierType> 
            exp(7, HyperNode::NOT_FRONTIER), act(7, HyperNode::IS_FRONTIER);
        exp[0] = HyperNode::IS_FRONTIER;
        exp[4] = HyperNode::IS_FRONTIER;
        exp[5] = HyperNode::IS_FRONTIER;
        src3_graph->CalculateFrontiers(src_span);
        for(int i = 0; i < 7; i++)
            act[i] = src3_graph->GetNode(i)->GetFrontier();
        return CheckVector(exp, act);
    }

    int TestCalculateFrontierForest() {
        vector<set<int> > src_span = align2.GetSrcAlignments();
        vector<set<int>*> node_span(src2_graph->NumNodes(), (set<int>*)NULL);
        // Here, we will treat terminal nodes as non-frontier, as we just
        vector<HyperNode::FrontierType> exp(10, HyperNode::IS_FRONTIER), act(10);
        exp[6] = HyperNode::NOT_FRONTIER; exp[9] = HyperNode::NOT_FRONTIER;
        src2_graph->CalculateFrontiers(src_span);
        for(int i = 0; i < 10; i++)
            act[i] = src2_graph->GetNode(i)->GetFrontier();
        return CheckVector(exp, act);
    }

    int TestNbestPath() {
        HyperGraph rule_graph;
        // Add the nodes
        HyperNode * n0 = new HyperNode; rule_graph.AddNode(n0);
        HyperNode * n1 = new HyperNode; rule_graph.AddNode(n1);
        HyperNode * n2 = new HyperNode; rule_graph.AddNode(n2);
        // Add the edges
        HyperEdge * e0 = new HyperEdge(n0); rule_graph.AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); n0->AddEdge(e0);
        HyperEdge * e1 = new HyperEdge(n0); rule_graph.AddEdge(e1); e1->AddTail(n1); e1->AddTail(n2); e1->SetScore(-0.7); n0->AddEdge(e1);
        HyperEdge * e2 = new HyperEdge(n1); rule_graph.AddEdge(e2); e2->SetScore(-0.1); n1->AddEdge(e2);
        HyperEdge * e3 = new HyperEdge(n1); rule_graph.AddEdge(e3); e3->SetScore(-0.3); n1->AddEdge(e3);
        HyperEdge * e4 = new HyperEdge(n2); rule_graph.AddEdge(e4); e4->SetScore(-0.2); n2->AddEdge(e4);
        HyperEdge * e5 = new HyperEdge(n2); rule_graph.AddEdge(e5); e5->SetScore(-0.5); n2->AddEdge(e5);
        // Accumulate Viterbi scores over nodes
        rule_graph.ResetViterbiScores();
        // The viterbi scores should be -0.6, -0.1, -0.2
        vector<double> exp_scores(3), act_scores(3);
        exp_scores[0] = -0.6; exp_scores[1] = -0.1; exp_scores[2] = -0.2;
        for(int i = 0; i < 3; i++)
            act_scores[i] = rule_graph.GetNode(i)->GetViterbiScore();
        return CheckAlmostVector(exp_scores, act_scores); 
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestCopy()" << endl; if(TestCopy()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateSpan()" << endl; if(TestCalculateSpan()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateSpanForest()" << endl; if(TestCalculateSpanForest()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateNull()" << endl; if(TestCalculateNull()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateFrontier()" << endl; if(TestCalculateFrontier()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateFrontierForest()" << endl; if(TestCalculateFrontierForest()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestNbestPath()" << endl; if(TestNbestPath()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestHyperGraph Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
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
