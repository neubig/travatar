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
        string src1_tree = "(S (NP (PRP he)) (VP (AUX does) (RB not) (VB go)))";
        string trg1_str  = "il ne va pas";
        string align1_str = "0-0 1-1 1-3 2-1 2-3 3-2";
        istringstream iss(src1_tree);
        src1_graph.reset(tree_io.ReadTree(iss));
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
    }

    ~TestRuleExtractor() { }

    int TestCalculateSpan() {
        ForestExtractor ghkm_ext;
        vector<set<int> > src_span = align1.GetSrcAlignments();
        vector<set<int>*> node_span(src1_graph->NumNodes(), (set<int>*)NULL);
        ghkm_ext.CalculateSpan(src1_graph->GetNode(0), src_span, node_span);
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
            if(node_exp[i] != *SafeAccess(node_span, i)) {
                cerr << i << " not equal" << endl;
                ret = 0;
            }
        return ret;
    }

    int TestCalculateSpanForest() {
        ForestExtractor ghkm_ext;
        vector<set<int> > src_span = align2.GetSrcAlignments();
        vector<set<int>*> node_span(src2_graph->NumNodes(), (set<int>*)NULL);
        for(int i = 0; i < 10; i++)
            ghkm_ext.CalculateSpan(src2_graph->GetNode(i), src_span, node_span);
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
            if(node_exp[i] != SafeReference(SafeAccess(node_span, i))) {
                ret = 0;
                cerr << i << " not equal " << SafeReference(SafeAccess(node_span, i)) << endl;
            }
        return ret;
    }

    int TestCalculateFrontier() {
        ForestExtractor ghkm_ext;
        vector<set<int> > src_span = align1.GetSrcAlignments();
        vector<set<int>*> node_span(src1_graph->NumNodes(), (set<int>*)NULL);
        ghkm_ext.CalculateFrontier(src1_graph->GetNode(0), src_span, node_span, set<int>());
        // Here, we will treat terminal nodes as non-frontier, as we just
        vector<HyperNode::FrontierType> 
            exp(11, HyperNode::IS_FRONTIER), act(11, HyperNode::IS_FRONTIER);
        exp[3] = HyperNode::NOT_FRONTIER;
        exp[5] = HyperNode::NOT_FRONTIER; exp[6] = HyperNode::NOT_FRONTIER;
        exp[7] = HyperNode::NOT_FRONTIER; exp[8] = HyperNode::NOT_FRONTIER;
        exp[10] = HyperNode::NOT_FRONTIER;
        for(int i = 0; i < 11; i++)
            act[i] = src1_graph->GetNode(i)->IsFrontier();
        return CheckVector(exp, act);
    }

    int TestCalculateFrontierForest() {
        ForestExtractor ghkm_ext;
        vector<set<int> > src_span = align2.GetSrcAlignments();
        vector<set<int>*> node_span(src2_graph->NumNodes(), (set<int>*)NULL);
        ghkm_ext.CalculateFrontier(src2_graph->GetNode(0), src_span, node_span, set<int>());
        // Here, we will treat terminal nodes as non-frontier, as we just
        vector<HyperNode::FrontierType> exp(10, HyperNode::IS_FRONTIER), act(10);
        exp[6] = HyperNode::NOT_FRONTIER; exp[9] = HyperNode::NOT_FRONTIER;
        for(int i = 0; i < 10; i++)
            act[i] = src2_graph->GetNode(i)->IsFrontier();
        return CheckVector(exp, act);
    }

    int TestTreeExtraction() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        vector<shared_ptr<GraphFragment> > frags_act = forest_ext.ExtractRules(*src1_graph, trg1_sent, align1);
        // Create the actual rule graph
        vector<shared_ptr<GraphFragment> > frags_exp(5);
        // Expected edge numbers: "(S0 (NP1 (PRP2 he)) (VP3 (AUX4 does) (RB5 not) (VB6 go)))";
        // Edge rooted at S0
        frags_exp[0].reset(new GraphFragment);
        frags_exp[0]->AddEdge(src1_graph->GetEdge(0));
        // Edge rooted at NP1
        frags_exp[1].reset(new GraphFragment);
        frags_exp[1]->AddEdge(src1_graph->GetEdge(1));
        // Edge rooted at PRP2
        frags_exp[2].reset(new GraphFragment);
        frags_exp[2]->AddEdge(src1_graph->GetEdge(2));
        // Edge rooted at VP3
        frags_exp[3].reset(new GraphFragment);
        frags_exp[3]->AddEdge(src1_graph->GetEdge(3));
        frags_exp[3]->AddEdge(src1_graph->GetEdge(4));
        frags_exp[3]->AddEdge(src1_graph->GetEdge(5));
        // Edge rooted at VB6
        frags_exp[4].reset(new GraphFragment);
        frags_exp[4]->AddEdge(src1_graph->GetEdge(6));
        // Check to make sure that these are equal
        return CheckPtrVector(frags_exp, frags_act);
    }

    int TestForestExtraction() {
        // Run the Forest algorithm
        ForestExtractor forest_ext;
        src2_graph->NormalizeEdgeProbabilities();
        vector<shared_ptr<GraphFragment> > frags_act = forest_ext.ExtractRules(*src2_graph, trg2_sent, align2);
        // Create the actual values, all non-terminal edges should be fine, and most should have
        // a probability of 0.5, as they only belong to one of the two trees
        vector<shared_ptr<GraphFragment> > frags_exp(9);
        for(int i = 0; i < 9; i++) {
            frags_exp[i].reset(new GraphFragment);
            frags_exp[i]->AddEdge(src2_graph->GetEdge(i));
            frags_exp[i]->SetProb(1);
        }
        // The only two edges that are 0.5 are the split edges on top
        frags_exp[0]->SetProb(0.5); frags_exp[1]->SetProb(0.5);
        // Check to make sure that these are equal
        return CheckPtrVector(frags_exp, frags_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestCalculateSpan()" << endl; if(TestCalculateSpan()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateSpanForest()" << endl; if(TestCalculateSpanForest()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateFrontier()" << endl; if(TestCalculateFrontier()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateFrontierForest()" << endl; if(TestCalculateFrontierForest()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestTreeExtraction()" << endl; if(TestTreeExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestForestExtraction()" << endl; if(TestForestExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestRuleExtractor Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    PennTreeIO tree_io;
    boost::scoped_ptr<HyperGraph> src1_graph, src2_graph;
    Sentence trg1_sent, trg2_sent;
    Alignment align1, align2;

};

}

#endif
