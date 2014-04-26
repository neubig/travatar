#ifndef TEST_HYPER_GRAPH_H__
#define TEST_HYPER_GRAPH_H__

#include <fstream>
#include <utility>
#include <boost/shared_ptr.hpp>
#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/translation-rule.h>

using namespace boost;
using namespace std;

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
        HyperNode* node0 = new HyperNode(Dict::WID("ROOT"), -1,    make_pair(0,2)); src2_graph->AddNode(node0);
        HyperNode* node1 = new HyperNode(Dict::WID("VP"), -1,      make_pair(0,2)); src2_graph->AddNode(node1);
        HyperNode* node2 = new HyperNode(Dict::WID("NP"), -1,      make_pair(0,2)); src2_graph->AddNode(node2);
        HyperNode* node3 = new HyperNode(Dict::WID("JJ"), -1,      make_pair(0,1)); src2_graph->AddNode(node3);
        HyperNode* node4 = new HyperNode(Dict::WID("VP"), -1,      make_pair(0,1)); src2_graph->AddNode(node4);
        HyperNode* node5 = new HyperNode(Dict::WID("VPG"), -1,     make_pair(0,1)); src2_graph->AddNode(node5);
        HyperNode* node6 = new HyperNode(Dict::WID("running"), -1, make_pair(0,1)); src2_graph->AddNode(node6);
        HyperNode* node7 = new HyperNode(Dict::WID("NP"), -1,      make_pair(1,2)); src2_graph->AddNode(node7);
        HyperNode* node8 = new HyperNode(Dict::WID("NN"), -1,      make_pair(1,2)); src2_graph->AddNode(node8);
        HyperNode* node9 = new HyperNode(Dict::WID("water"), -1,   make_pair(1,2)); src2_graph->AddNode(node9);
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
        HyperNode * n0 = new HyperNode; n0->SetSpan(make_pair(0,2)); rule_graph_->AddNode(n0);
        HyperNode * n1 = new HyperNode; n1->SetSpan(make_pair(0,1)); rule_graph_->AddNode(n1);
        HyperNode * n2 = new HyperNode; n2->SetSpan(make_pair(1,2)); rule_graph_->AddNode(n2);
        rule_01.reset(new TranslationRule); rule_01->AddTrgWord(-1); rule_01->AddTrgWord(-2);
        HyperEdge * e0 = new HyperEdge(n0); rule_graph_->AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); e0->SetRule(rule_01.get()); n0->AddEdge(e0);
        e0->AddFeature(Dict::WID("toy_feature"), 1.5);
        rule_10.reset(new TranslationRule); rule_10->AddTrgWord(-2); rule_10->AddTrgWord(-1);
        HyperEdge * e1 = new HyperEdge(n0); rule_graph_->AddEdge(e1); e1->AddTail(n1); e1->AddTail(n2); e1->SetScore(-0.7); e1->SetRule(rule_10.get()); n0->AddEdge(e1);
        rule_a.reset(new TranslationRule); rule_a->AddTrgWord(Dict::WID("a")); rule_a->AddTrgWord(Dict::WID("b"));
        HyperEdge * e2 = new HyperEdge(n1); rule_graph_->AddEdge(e2); e2->SetScore(-0.1); e2->SetRule(rule_a.get()); n1->AddEdge(e2);
        rule_b.reset(new TranslationRule); rule_b->AddTrgWord(Dict::WID("a")); rule_b->AddTrgWord(Dict::WID("c"));
        HyperEdge * e3 = new HyperEdge(n1); rule_graph_->AddEdge(e3); e3->SetScore(-0.3); e3->SetRule(rule_b.get()); n1->AddEdge(e3);
        rule_x.reset(new TranslationRule); rule_x->AddTrgWord(Dict::WID("x"));
        HyperEdge * e4 = new HyperEdge(n2); rule_graph_->AddEdge(e4); e4->SetScore(-0.2); e4->SetRule(rule_x.get()); n2->AddEdge(e4);
        rule_y.reset(new TranslationRule); rule_y->AddTrgWord(Dict::WID("y"));
        HyperEdge * e5 = new HyperEdge(n2); rule_graph_->AddEdge(e5); e5->SetScore(-0.5); e5->SetRule(rule_y.get()); n2->AddEdge(e5);
        rule_unk.reset(new TranslationRule); rule_unk->AddTrgWord(Dict::WID("t"));
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

    int TestInsideOutside() {
        HyperGraph src2_copy(*src2_graph);
        src2_copy.InsideOutsideNormalize();
        vector<double> prob_exp(9,log(.5)), prob_act;
        prob_exp[8] = 0;
        BOOST_FOREACH(const HyperEdge * edge, src2_copy.GetEdges())
            prob_act.push_back(edge->GetScore());
        return CheckAlmostVector(prob_exp, prob_act);
    }

    int TestInsideOutsideUnbalanced() {
        // Build the initial graph
        HyperGraph hg_act;
        HyperNode* node0 = new HyperNode(Dict::WID("N0"), -1, make_pair(0,3)); hg_act.AddNode(node0);
        HyperNode* node1 = new HyperNode(Dict::WID("N1"), -1, make_pair(0,2)); hg_act.AddNode(node1);
        HyperNode* node2 = new HyperNode(Dict::WID("N2"), -1, make_pair(2,3)); hg_act.AddNode(node2);
        HyperNode* node3 = new HyperNode(Dict::WID("N3"), -1, make_pair(0,2)); hg_act.AddNode(node3);
        HyperNode* node4 = new HyperNode(Dict::WID("N4"), -1, make_pair(0,1)); hg_act.AddNode(node4);
        HyperNode* node5 = new HyperNode(Dict::WID("N5"), -1, make_pair(1,2)); hg_act.AddNode(node5);
        HyperNode* node6 = new HyperNode(Dict::WID("N6"), -1, make_pair(0,1)); hg_act.AddNode(node6);
        HyperNode* node7 = new HyperNode(Dict::WID("N7"), -1, make_pair(1,2)); hg_act.AddNode(node7);
        HyperEdge* edge0 = new HyperEdge(node0); edge0->AddTail(node1); edge0->AddTail(node2); node0->AddEdge(edge0); hg_act.AddEdge(edge0);
        HyperEdge* edge1 = new HyperEdge(node0); edge1->AddTail(node3); edge1->AddTail(node2); node0->AddEdge(edge1); hg_act.AddEdge(edge1);
        HyperEdge* edge2 = new HyperEdge(node3); edge2->AddTail(node4); edge2->AddTail(node5); node3->AddEdge(edge2); hg_act.AddEdge(edge2);
        HyperEdge* edge3 = new HyperEdge(node3); edge3->AddTail(node6); edge3->AddTail(node7); node3->AddEdge(edge3); hg_act.AddEdge(edge3);
        HyperEdge* edge4 = new HyperEdge(node3); edge4->AddTail(node4); edge4->AddTail(node7); node3->AddEdge(edge4); hg_act.AddEdge(edge4);
        // Perform the inside outside algorithm to normalize
        hg_act.InsideOutsideNormalize();
        // Get the scores of both graphs
        vector<double> score_exp(5), score_act(5);
        score_exp[0] = log(0.25); score_act[0] = edge0->GetScore();
        score_exp[1] = log(0.75); score_act[1] = edge1->GetScore();
        score_exp[2] = log(0.25); score_act[2] = edge2->GetScore();
        score_exp[3] = log(0.25); score_act[3] = edge3->GetScore();
        score_exp[4] = log(0.25); score_act[4] = edge4->GetScore();
        return ApproximateDoubleEquals(score_exp, score_act);
    }

    int TestCopy() {
        HyperGraph src1_copy(*src1_graph);
        int ret = src1_graph->CheckEqual(src1_copy);
        if(ret) {
            const vector<HyperNode*> & old_nodes = src1_graph->GetNodes();
            const vector<HyperNode*> & new_nodes = src1_copy.GetNodes();
            for(int i = 0; i < (int)old_nodes.size(); i++) {
                if(old_nodes[i] == new_nodes[i]) {
                    cerr << "old and new node pointers are equal @ " << i << endl;
                    ret = 0;
                } else {
                    const vector<HyperEdge*> & old_edges = old_nodes[i]->GetEdges();
                    const vector<HyperEdge*> & new_edges = new_nodes[i]->GetEdges();
                    for(int j = 0; j < (int)old_edges.size(); j++) {
                        if(old_edges[j] == new_edges[j]) {
                            cerr << "old and new edge pointers are equal @ " << i << ", " << j << endl;
                            ret = 0;
                        }
                    }
                }
            }
            const vector<HyperEdge*> & old_edges = src1_graph->GetEdges();
            const vector<HyperEdge*> & new_edges = src1_copy.GetEdges();
            for(int i = 0; i < (int)old_edges.size(); i++) {
                if(old_edges[i] == new_edges[i]) {
                    cerr << "old and new edge pointers are equal @ " << i << endl;
                    ret = 0;
                } else {
                    const vector<HyperNode*> & old_tails = old_edges[i]->GetTails();
                    const vector<HyperNode*> & new_tails = new_edges[i]->GetTails();
                    for(int j = 0; j < (int)old_tails.size(); j++) {
                        if(old_tails[j] == new_tails[j]) {
                            cerr << "old and new tail pointers are equal @ " << i << ", " << j << endl;
                            ret = 0;
                        }
                    }
                }
            }
        }
        return ret;
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
            act_scores[i] = rule_graph_->GetNode(i)->CalcViterbiScore();
        if(!CheckAlmostVector(exp_scores, act_scores))
            return false;
        // Get the three-best edge values
        vector<shared_ptr<HyperPath> > exp_nbest, act_nbest;
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[0]->AddEdge(rule_graph_->GetEdge(0)); exp_nbest[0]->AddEdge(rule_graph_->GetEdge(2)); exp_nbest[0]->AddEdge(rule_graph_->GetEdge(4)); exp_nbest[0]->SetScore(-0.6);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[1]->AddEdge(rule_graph_->GetEdge(0)); exp_nbest[1]->AddEdge(rule_graph_->GetEdge(3)); exp_nbest[1]->AddEdge(rule_graph_->GetEdge(4)); exp_nbest[1]->SetScore(-0.8);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[2]->AddEdge(rule_graph_->GetEdge(0)); exp_nbest[2]->AddEdge(rule_graph_->GetEdge(2)); exp_nbest[2]->AddEdge(rule_graph_->GetEdge(5)); exp_nbest[2]->SetScore(-0.9);
        act_nbest = rule_graph_->GetNbest(3, rule_graph_->GetWords());
        return CheckPtrVector(exp_nbest, act_nbest);
    }

    // Test the ordering of the n-best list when all hypotheses are tied
    // The ordering should fall back to orthographic order of edge IDs
    int TestNbestTied() {
        // Create a new copy of the rule graph where all scores are tied
        boost::scoped_ptr<HyperGraph> tied_graph(new HyperGraph(*rule_graph_));
        BOOST_FOREACH(HyperEdge* e, tied_graph->GetEdges())
            e->SetScore(0.0);
        // Accumulate Viterbi scores over nodes
        tied_graph->ResetViterbiScores();
        vector<double> exp_scores(3), act_scores(3);
        exp_scores[0] = 0; exp_scores[1] = 0; exp_scores[2] = 0;
        for(int i = 0; i < 3; i++)
            act_scores[i] = tied_graph->GetNode(i)->CalcViterbiScore();
        if(!CheckAlmostVector(exp_scores, act_scores))
            return false;
        // Get the three-best n-best lists (a b x) (a b y) (a c x)
        vector<shared_ptr<HyperPath> > exp_nbest, act_nbest;
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[0]->AddEdge(tied_graph->GetEdge(0)); exp_nbest[0]->AddEdge(tied_graph->GetEdge(2)); exp_nbest[0]->AddEdge(tied_graph->GetEdge(4)); exp_nbest[0]->SetScore(0);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[1]->AddEdge(tied_graph->GetEdge(0)); exp_nbest[1]->AddEdge(tied_graph->GetEdge(2)); exp_nbest[1]->AddEdge(tied_graph->GetEdge(5)); exp_nbest[1]->SetScore(0);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[2]->AddEdge(tied_graph->GetEdge(0)); exp_nbest[2]->AddEdge(tied_graph->GetEdge(2)); exp_nbest[2]->AddEdge(tied_graph->GetEdge(6)); exp_nbest[2]->SetScore(0);
        exp_nbest.push_back(shared_ptr<HyperPath>(new HyperPath)); exp_nbest[3]->AddEdge(tied_graph->GetEdge(0)); exp_nbest[3]->AddEdge(tied_graph->GetEdge(3)); exp_nbest[3]->AddEdge(tied_graph->GetEdge(4)); exp_nbest[3]->SetScore(0);
        act_nbest = tied_graph->GetNbest(4, tied_graph->GetWords());
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
        exp_trans[0] = "a b x";
        exp_trans[1] = "a c x";
        exp_trans[2] = "y a b";
        exp_trans[3] = "t a b";
        for(int i = 0; i < 4; i++)
            act_trans[i] = Dict::PrintWords(paths[i]->CalcTranslation(0).words);
        return CheckVector(exp_trans, act_trans);
    }

//    int TestLMIntersection() {
//
//        string file_name = "/tmp/test-hyper-graph-lm.arpa";
//        ofstream arpa_out(file_name.c_str());
//        arpa_out << ""
//"\\data\\\n"
//"ngram 1=7\n"
//"ngram 2=8\n"
//"\n"
//"\\1-grams:\n"
//"-0.6368221	</s>\n"
//"-99	<s>	-0.30103\n"
//"-0.6368221	a	-0.4771213\n"
//"-0.6368221	b	-0.30103\n"
//"-0.6368221	c	-0.30103\n"
//"-0.8129134	x	-0.30103\n"
//"-0.8129134	y	-0.30103\n"
//"\n"
//"\\2-grams:\n"
//"-0.4372497	<s> a\n"
//"-0.4855544	<s> y\n"
//"-0.1286666	a b\n"
//"-0.1286666	a c\n"
//"-0.4372497	b </s>\n"
//"-0.4855544	b x\n"
//"-0.2108534	x </s>\n"
//"-0.2108534	y a" 
//"\n"
//"\\end\\\n" << endl;
//        arpa_out.close();
//
//        // Intersect the graph with the LM
//        LMComposerBU lm(new lm::ngram::Model(file_name.c_str()));
//        lm.SetStackPopLimit(3);
//        shared_ptr<HyperGraph> exp_graph(new HyperGraph), act_graph(lm.TransformGraph(*rule_graph_));
//
//        // Create the expected graph
//        vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
//        exp_graph->SetWords(ab);
//        // The root node should be "0,2"
//        HyperNode * n_root = new HyperNode; n_root->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_root);
//        n_root->SetSym(Dict::WID("LMROOT"));
//
//        // HyperEdge * e0 = new HyperEdge(n0); rule_graph_->AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); e0->SetRule(rule_01.get()); n0->AddEdge(e0);
//        // e0->AddFeature(Dict::WID("toy_feature"), 1.5);
//        // rule_10.reset(new TranslationRule); rule_10->AddTrgWord(-2); rule_10->AddTrgWord(-1);
//        // HyperEdge * e1 = new HyperEdge(n0); rule_graph_->AddEdge(e1); e1->AddTail(n1); e1->AddTail(n2); e1->SetScore(-0.7); e1->SetRule(rule_10.get()); n0->AddEdge(e1);
//        // rule_a.reset(new TranslationRule); rule_a->AddTrgWord(Dict::WID("a")); rule_a->AddTrgWord(Dict::WID("b"));
//        // HyperEdge * e2 = new HyperEdge(n1); rule_graph_->AddEdge(e2); e2->SetScore(-0.1); e2->SetRule(rule_a.get()); n1->AddEdge(e2);
//        // rule_b.reset(new TranslationRule); rule_b->AddTrgWord(Dict::WID("a")); rule_b->AddTrgWord(Dict::WID("c"));
//        // HyperEdge * e3 = new HyperEdge(n1); rule_graph_->AddEdge(e3); e3->SetScore(-0.3); e3->SetRule(rule_b.get()); n1->AddEdge(e3);
//        // rule_x.reset(new TranslationRule); rule_x->AddTrgWord(Dict::WID("x"));
//        // HyperEdge * e4 = new HyperEdge(n2); rule_graph_->AddEdge(e4); e4->SetScore(-0.2); e4->SetRule(rule_x.get()); n2->AddEdge(e4);
//        // rule_y.reset(new TranslationRule); rule_y->AddTrgWord(Dict::WID("y"));
//        // HyperEdge * e5 = new HyperEdge(n2); rule_graph_->AddEdge(e5); e5->SetScore(-0.5); e5->SetRule(rule_y.get()); n2->AddEdge(e5);
//            // rule_unk.reset(new TranslationRule); rule_unk->AddTrgWord(Dict::WID("<unk>"));
//            // HyperEdge * e6 = new HyperEdge(n2); rule_graph_->AddEdge(e6); e6->SetScore(-2.5); e6->SetRule(rule_unk.get()); n2->AddEdge(e6);
//        // Start on options for the left node, there should be two nodes for "a*b" and "a*c"
//        HyperNode * n_01_ab = new HyperNode; n_01_ab->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ab);
//        n_01_ab->SetViterbiScore(-0.1286666 + -0.6368221 + -0.1);
//        HyperNode * n_01_ac = new HyperNode; n_01_ac->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ac);
//        n_01_ac->SetViterbiScore(-0.1286666 + -0.6368221 + -0.3);
//        // Options on the right node should be "x" and "y"
//        HyperNode * n_12_x = new HyperNode; n_12_x->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_x);
//        n_12_x->SetViterbiScore(-0.8129134 + -0.2);
//        HyperNode * n_12_y = new HyperNode; n_12_y->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_y);
//        n_12_y->SetViterbiScore(-0.8129134 + -0.5);
//        HyperNode * n_12_t = new HyperNode; n_12_t->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_t);
//        n_12_t->SetViterbiScore(-100 + -2.5);
//        // Options on the top node include a*x, a*y, x*b, x*c, y*b, y*c
//        HyperNode * n_02_ax = new HyperNode; n_02_ax->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ax);
//        n_02_ax->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_x->GetViterbiScore() + -0.3 + -0.4855544 - -0.8129134);
//        HyperNode * n_02_ay = new HyperNode; n_02_ay->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ay);
//        n_02_ay->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.3 + -0.30103);
//        n_root->SetViterbiScore(n_02_ax->GetViterbiScore() + -0.4372497 - -0.6368221 + -0.2108534);
//        // Make edges for 0,1. There are only 2, so no pruning
//        HyperEdge * e_01_ab = new HyperEdge(n_01_ab); exp_graph->AddEdge(e_01_ab); n_01_ab->AddEdge(e_01_ab);
//        e_01_ab->SetFeatures(rule_graph_->GetEdge(2)->GetFeatures()); e_01_ab->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
//        e_01_ab->SetTrgWords(rule_graph_->GetEdge(2)->GetTrgWords());
//        HyperEdge * e_01_ac = new HyperEdge(n_01_ac); exp_graph->AddEdge(e_01_ac); n_01_ac->AddEdge(e_01_ac);
//        e_01_ac->SetFeatures(rule_graph_->GetEdge(3)->GetFeatures()); e_01_ac->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
//        e_01_ac->SetTrgWords(rule_graph_->GetEdge(3)->GetTrgWords());
//        // Make edges for 1,2. There are only 3, so no pruning
//        HyperEdge * e_12_x = new HyperEdge(n_12_x); exp_graph->AddEdge(e_12_x); n_12_x->AddEdge(e_12_x);
//        e_12_x->SetFeatures(rule_graph_->GetEdge(4)->GetFeatures()); e_12_x->AddFeature(Dict::WID("lm"), -0.8129134); // P(x)
//        e_12_x->SetTrgWords(rule_graph_->GetEdge(4)->GetTrgWords());
//        HyperEdge * e_12_y = new HyperEdge(n_12_y); exp_graph->AddEdge(e_12_y); n_12_y->AddEdge(e_12_y);
//        e_12_y->SetFeatures(rule_graph_->GetEdge(5)->GetFeatures()); e_12_y->AddFeature(Dict::WID("lm"), -0.8129134); // P(y)
//        e_12_y->SetTrgWords(rule_graph_->GetEdge(5)->GetTrgWords());
//        HyperEdge * e_12_t = new HyperEdge(n_12_t); exp_graph->AddEdge(e_12_t); n_12_t->AddEdge(e_12_t);
//        e_12_t->SetFeatures(rule_graph_->GetEdge(6)->GetFeatures()); e_12_t->AddFeature(Dict::WID("lm"), -100); // P(unk)
//        e_12_t->SetTrgWords(rule_graph_->GetEdge(6)->GetTrgWords());
//        // Make edges for 0,2. There are more than three, so only expand the best three
//        HyperEdge * e_02_abx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_abx); n_02_ax->AddEdge(e_02_abx);
//        e_02_abx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures()); e_02_abx->AddFeature(Dict::WID("lm"), -0.4855544 - -0.8129134);
//        e_02_abx->AddTail(n_01_ab); e_02_abx->AddTail(n_12_x);
//        e_02_abx->SetTrgWords(rule_graph_->GetEdge(0)->GetTrgWords());
//        HyperEdge * e_02_acx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_acx); n_02_ax->AddEdge(e_02_acx);
//        e_02_acx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures()); e_02_acx->AddFeature(Dict::WID("lm"), -0.30103);
//        e_02_acx->AddTail(n_01_ac); e_02_acx->AddTail(n_12_x);
//        e_02_acx->SetTrgWords(rule_graph_->GetEdge(0)->GetTrgWords());
//        HyperEdge * e_02_aby = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_aby); n_02_ay->AddEdge(e_02_aby);
//        e_02_aby->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures()); e_02_aby->AddFeature(Dict::WID("lm"), -0.30103);
//        e_02_aby->AddTail(n_01_ab); e_02_aby->AddTail(n_12_y);
//        e_02_aby->SetTrgWords(rule_graph_->GetEdge(0)->GetTrgWords());
//        // Make edges for the root. There are only two
//        HyperEdge * e_root_ax = new HyperEdge(n_root); exp_graph->AddEdge(e_root_ax); n_root->AddEdge(e_root_ax);
//        e_root_ax->AddFeature(Dict::WID("lm"), -0.4372497 - -0.6368221 + -0.2108534);
//        e_root_ax->AddTail(n_02_ax);
//        e_root_ax->AddTrgWord(-1);
//        HyperEdge * e_root_ay = new HyperEdge(n_root); exp_graph->AddEdge(e_root_ay); n_root->AddEdge(e_root_ay);
//        e_root_ay->AddFeature(Dict::WID("lm"), -0.4372497 - -0.6368221 + -0.30103 + -0.6368221);
//        e_root_ay->AddTail(n_02_ay);
//        e_root_ay->AddTrgWord(-1);
//        return exp_graph->CheckEqual(*act_graph);
//    }

    int TestAppend() {
        // also an example of a forest
        HyperGraph hga, hgb, hgab_exp;
        { 
            HyperNode* nodeA1 = new HyperNode(Dict::WID("A1"), -1, make_pair(0,1)); hga.AddNode(nodeA1);
            HyperNode* nodeA2 = new HyperNode(Dict::WID("A2"), -1, make_pair(0,1)); hga.AddNode(nodeA2);
            HyperEdge* edgeA = new HyperEdge(nodeA1); edgeA->AddTail(nodeA2); nodeA2->AddEdge(edgeA); hga.AddEdge(edgeA);
            HyperNode* nodeB1 = new HyperNode(Dict::WID("B1"), -1, make_pair(0,1)); hgb.AddNode(nodeB1);
            HyperNode* nodeB2 = new HyperNode(Dict::WID("B2"), -1, make_pair(0,1)); hgb.AddNode(nodeB2);
            HyperEdge* edgeB = new HyperEdge(nodeB1); edgeB->AddTail(nodeB2); nodeB2->AddEdge(edgeB); hgb.AddEdge(edgeB);
            hga.Append(hgb);
        }
        { 
            HyperNode* nodeA1 = new HyperNode(Dict::WID("A1"), -1, make_pair(0,1)); hgab_exp.AddNode(nodeA1);
            HyperNode* nodeA2 = new HyperNode(Dict::WID("A2"), -1, make_pair(0,1)); hgab_exp.AddNode(nodeA2);
            HyperEdge* edgeA = new HyperEdge(nodeA1); edgeA->AddTail(nodeA2); nodeA2->AddEdge(edgeA); hgab_exp.AddEdge(edgeA);
            HyperNode* nodeB1 = new HyperNode(Dict::WID("B1"), -1, make_pair(0,1)); hgab_exp.AddNode(nodeB1);
            HyperNode* nodeB2 = new HyperNode(Dict::WID("B2"), -1, make_pair(0,1)); hgab_exp.AddNode(nodeB2);
            HyperEdge* edgeB = new HyperEdge(nodeB1); edgeB->AddTail(nodeB2); nodeB2->AddEdge(edgeB); hgab_exp.AddEdge(edgeB);
        }
        return hgab_exp.CheckEqual(hga);
    }

    // See if labeling spans works
    int TestGetLabeledSpans() {
        string src1_tree = "(S (NP (PRP he)) (VP (AUX does) (RB not) (VB go)))";
        LabeledSpans exp_spans, act_spans = src1_graph->GetLabeledSpans();
        exp_spans[make_pair(0,4)] = Dict::WID("S");
        exp_spans[make_pair(0,1)] = Dict::WID("NP");
        exp_spans[make_pair(1,4)] = Dict::WID("VP");
        exp_spans[make_pair(1,2)] = Dict::WID("AUX");
        exp_spans[make_pair(2,3)] = Dict::WID("RB");
        exp_spans[make_pair(3,4)] = Dict::WID("VB");
        return CheckMap(exp_spans, act_spans);
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
        done++; cout << "TestNbestTied()" << endl; if(TestNbestTied()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestPathTranslation()" << endl; if(TestPathTranslation()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestInsideOutside()" << endl; if(TestInsideOutside()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestInsideOutsideUnbalanced()" << endl; if(TestInsideOutsideUnbalanced()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestAppend()" << endl; if(TestAppend()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestGetLabeledSpans()" << endl; if(TestGetLabeledSpans()) succeeded++; else cout << "FAILED!!!" << endl;
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
