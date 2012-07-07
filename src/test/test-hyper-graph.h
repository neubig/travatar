#ifndef TEST_HYPER_GRAPH_H__
#define TEST_HYPER_GRAPH_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/translation-rule.h>
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
        // Example rule graph
        rule_graph_.reset(new HyperGraph);
        vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
        rule_graph_->SetWords(ab);
        HyperNode * n0 = new HyperNode; n0->SetSpan(MakePair(0,2)); rule_graph_->AddNode(n0);
        HyperNode * n1 = new HyperNode; n1->SetSpan(MakePair(0,1)); rule_graph_->AddNode(n1);
        HyperNode * n2 = new HyperNode; n2->SetSpan(MakePair(1,2)); rule_graph_->AddNode(n2);
        rule_01.reset(new TranslationRule); rule_01->AddTrgWord(-1); rule_01->AddTrgWord(-2);
        HyperEdge * e0 = new HyperEdge(n0); rule_graph_->AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); e0->SetRule(rule_01.get()); n0->AddEdge(e0);
        rule_10.reset(new TranslationRule); rule_10->AddTrgWord(-2); rule_10->AddTrgWord(-1);
        HyperEdge * e1 = new HyperEdge(n0); rule_graph_->AddEdge(e1); e1->AddTail(n1); e1->AddTail(n2); e1->SetScore(-0.7); e1->SetRule(rule_10.get()); n0->AddEdge(e1);
        rule_a.reset(new TranslationRule); rule_a->AddTrgWord(Dict::WID("a"));
        HyperEdge * e2 = new HyperEdge(n1); rule_graph_->AddEdge(e2); e2->SetScore(-0.1); e2->SetRule(rule_a.get()); n1->AddEdge(e2);
        rule_b.reset(new TranslationRule); rule_b->AddTrgWord(Dict::WID("b"));
        HyperEdge * e3 = new HyperEdge(n1); rule_graph_->AddEdge(e3); e3->SetScore(-0.3); e3->SetRule(rule_b.get()); n1->AddEdge(e3);
        rule_x.reset(new TranslationRule); rule_x->AddTrgWord(Dict::WID("x"));
        HyperEdge * e4 = new HyperEdge(n2); rule_graph_->AddEdge(e4); e4->SetScore(-0.2); e4->SetRule(rule_x.get()); n2->AddEdge(e4);
        rule_y.reset(new TranslationRule); rule_y->AddTrgWord(Dict::WID("y"));
        HyperEdge * e5 = new HyperEdge(n2); rule_graph_->AddEdge(e5); e5->SetScore(-0.5); e5->SetRule(rule_y.get()); n2->AddEdge(e5);
        rule_unk.reset(new TranslationRule); rule_unk->AddTrgWord(INT_MAX);
        HyperEdge * e6 = new HyperEdge(n2); rule_graph_->AddEdge(e6); e6->SetScore(-2.5); e6->SetRule(rule_unk.get()); n2->AddEdge(e6);
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
        // Accumulate Viterbi scores over nodes
        rule_graph_->ResetViterbiScores();
        // The viterbi scores should be -0.6, -0.1, -0.2
        vector<double> exp_scores(3), act_scores(3);
        exp_scores[0] = -0.6; exp_scores[1] = -0.1; exp_scores[2] = -0.2;
        for(int i = 0; i < 3; i++)
            act_scores[i] = rule_graph_->GetNode(i)->GetViterbiScore();
        if(!CheckAlmostVector(exp_scores, act_scores))
            return false;
        // Get the three-best edge values
        vector<shared_ptr<HyperPath> > exp_nbest, act_nbest;
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[0]->AddEdge(rule_graph_->GetEdge(0)); exp_nbest[0]->AddEdge(rule_graph_->GetEdge(2)); exp_nbest[0]->AddEdge(rule_graph_->GetEdge(4)); exp_nbest[0]->SetScore(-0.6);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[1]->AddEdge(rule_graph_->GetEdge(0)); exp_nbest[1]->AddEdge(rule_graph_->GetEdge(3)); exp_nbest[1]->AddEdge(rule_graph_->GetEdge(4)); exp_nbest[1]->SetScore(-0.8);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[2]->AddEdge(rule_graph_->GetEdge(0)); exp_nbest[2]->AddEdge(rule_graph_->GetEdge(2)); exp_nbest[2]->AddEdge(rule_graph_->GetEdge(5)); exp_nbest[2]->SetScore(-0.9);
        act_nbest = rule_graph_->GetNbest(3);
        return CheckPtrVector(exp_nbest, act_nbest);
    }

    int TestPathTranslation() {
        // Get the three-best edge values
        vector<shared_ptr<HyperPath> > paths;
        paths.push_back(shared_ptr<HyperPath>(new HyperPath)); paths[0]->AddEdge(rule_graph_->GetEdge(0)); paths[0]->AddEdge(rule_graph_->GetEdge(2)); paths[0]->AddEdge(rule_graph_->GetEdge(4)); paths[0]->SetScore(-0.6);
        paths.push_back(shared_ptr<HyperPath>(new HyperPath)); paths[1]->AddEdge(rule_graph_->GetEdge(0)); paths[1]->AddEdge(rule_graph_->GetEdge(3)); paths[1]->AddEdge(rule_graph_->GetEdge(4)); paths[1]->SetScore(-0.8);
        paths.push_back(shared_ptr<HyperPath>(new HyperPath)); paths[2]->AddEdge(rule_graph_->GetEdge(1)); paths[2]->AddEdge(rule_graph_->GetEdge(2)); paths[2]->AddEdge(rule_graph_->GetEdge(5)); paths[2]->SetScore(-0.9);
        paths.push_back(shared_ptr<HyperPath>(new HyperPath)); paths[3]->AddEdge(rule_graph_->GetEdge(1)); paths[3]->AddEdge(rule_graph_->GetEdge(2)); paths[3]->AddEdge(rule_graph_->GetEdge(6)); paths[3]->SetScore(-2.9);
        // Create the expected and actual values 
        vector<string> exp_trans(4), act_trans(4);
        exp_trans[0] = "a x";
        exp_trans[1] = "b x";
        exp_trans[2] = "y a";
        exp_trans[3] = "t a";
        for(int i = 0; i < 4; i++)
            act_trans[i] = Dict::PrintWords(paths[i]->CalcTranslation(rule_graph_->GetWords()));
        return CheckVector(exp_trans, act_trans);
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
        done++; cout << "TestPathTranslation()" << endl; if(TestPathTranslation()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestHyperGraph Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    PennTreeIO tree_io;
    boost::scoped_ptr<HyperGraph> src1_graph, src2_graph, src3_graph, rule_graph_;
    Sentence trg1_sent, trg2_sent, trg3_sent;
    Alignment align1, align2, align3;
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

}

#endif
