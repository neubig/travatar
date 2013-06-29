#ifndef TEST_GRAPH_TRANSFORMER_H__
#define TEST_GRAPH_TRANSFORMER_H__

#include "test-base.h"
#include <utility>
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/translation-rule.h>
#include <travatar/unary-flattener.h>
#include <travatar/word-splitter.h>
#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace std;

namespace travatar {

class TestGraphTransformer : public TestBase {

public:

    TestGraphTransformer() {
        // Example unary graph
        unary_graph_.reset(new HyperGraph);
        {
            src_.resize(1); src_[0] = Dict::WID("s");
            unary_graph_->SetWords(src_);
            HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,1));   unary_graph_->AddNode(na);  na->SetSym( Dict::WID("A" ));
            HyperNode * nb = new HyperNode; nb->SetSpan(make_pair(0,1)); unary_graph_->AddNode(nb); nb->SetSym(Dict::WID("B"));
            HyperEdge * e = new HyperEdge(na); unary_graph_->AddEdge(e); e->AddTail(nb); na->AddEdge(e);
        }
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
        rule_unk.reset(new TranslationRule); rule_unk->AddTrgWord(Dict::WID("<unk>"));
        HyperEdge * e6 = new HyperEdge(n2); rule_graph_->AddEdge(e6); e6->SetScore(-2.5); e6->SetRule(rule_unk.get()); n2->AddEdge(e6);
    }

    ~TestGraphTransformer() { }

    int TestLMIntersection() {

        string file_name = "/tmp/test-hyper-graph-lm.arpa";
        ofstream arpa_out(file_name.c_str());
        arpa_out << ""
"\\data\\\n"
"ngram 1=7\n"
"ngram 2=8\n"
"\n"
"\\1-grams:\n"
"-0.6368221	</s>\n"
"-99	<s>	-0.30103\n"
"-0.6368221	a	-0.4771213\n"
"-0.6368221	b	-0.30103\n"
"-0.6368221	c	-0.30103\n"
"-0.8129134	x	-0.30103\n"
"-0.8129134	y	-0.30103\n"
"\n"
"\\2-grams:\n"
"-0.4372497	<s> a\n"
"-0.4855544	<s> y\n"
"-0.1286666	a b\n"
"-0.1286666	a c\n"
"-0.4372497	b </s>\n"
"-0.4855544	b x\n"
"-0.2108534	x </s>\n"
"-0.2108534	y a" 
"\n"
"\\end\\\n" << endl;
        arpa_out.close();

        // Intersect the graph with the LM
        LMComposerBU lm(new lm::ngram::Model(file_name.c_str()));
        lm.SetStackPopLimit(3);
        shared_ptr<HyperGraph> exp_graph(new HyperGraph), act_graph(lm.TransformGraph(*rule_graph_));

        // Create the expected graph
        vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
        exp_graph->SetWords(ab);
        // The root node should be "0,2"
        HyperNode * n_root = new HyperNode; n_root->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_root);
        n_root->SetSym(Dict::WID("LMROOT"));
        HyperNode * n_01_ab = new HyperNode; n_01_ab->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ab);
        n_01_ab->SetViterbiScore(-0.1286666 + -0.6368221 + -0.1);
        HyperNode * n_01_ac = new HyperNode; n_01_ac->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ac);
        n_01_ac->SetViterbiScore(-0.1286666 + -0.6368221 + -0.3);
        // Options on the right node should be "x" and "y"
        HyperNode * n_12_x = new HyperNode; n_12_x->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_x);
        n_12_x->SetViterbiScore(-0.8129134 + -0.2);
        HyperNode * n_12_y = new HyperNode; n_12_y->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_y);
        n_12_y->SetViterbiScore(-0.8129134 + -0.5);
        HyperNode * n_12_t = new HyperNode; n_12_t->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_t);
        n_12_t->SetViterbiScore(-100 + -2.5);
        // Options on the top node include a*x, a*y, x*b, x*c, y*b, y*c
        HyperNode * n_02_ax = new HyperNode; n_02_ax->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ax);
        n_02_ax->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_x->GetViterbiScore() + -0.3 + -0.4855544 - -0.8129134);
        HyperNode * n_02_ay = new HyperNode; n_02_ay->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ay);
        n_02_ay->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.3 + -0.30103);
        n_root->SetViterbiScore(n_02_ax->GetViterbiScore() + -0.4372497 - -0.6368221 + -0.2108534);
        // Make edges for 0,1. There are only 2, so no pruning
        HyperEdge * e_01_ab = new HyperEdge(n_01_ab); exp_graph->AddEdge(e_01_ab); n_01_ab->AddEdge(e_01_ab);
        e_01_ab->SetFeatures(rule_graph_->GetEdge(2)->GetFeatures());
        e_01_ab->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
        e_01_ab->SetScore(-0.1286666 + -0.6368221 + -0.1);
        e_01_ab->SetTrgWords(rule_graph_->GetEdge(2)->GetTrgWords());
        HyperEdge * e_01_ac = new HyperEdge(n_01_ac); exp_graph->AddEdge(e_01_ac); n_01_ac->AddEdge(e_01_ac);
        e_01_ac->SetFeatures(rule_graph_->GetEdge(3)->GetFeatures());
        e_01_ac->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
        e_01_ac->SetScore(-0.1286666 + -0.6368221 + -0.3);
        e_01_ac->SetTrgWords(rule_graph_->GetEdge(3)->GetTrgWords());
        // Make edges for 1,2. There are only 3, so no pruning
        HyperEdge * e_12_x = new HyperEdge(n_12_x); exp_graph->AddEdge(e_12_x); n_12_x->AddEdge(e_12_x);
        e_12_x->SetFeatures(rule_graph_->GetEdge(4)->GetFeatures());
        e_12_x->AddFeature(Dict::WID("lm"), -0.8129134); // P(x)
        e_12_x->SetScore(-0.8129134 + -0.2);
        e_12_x->SetTrgWords(rule_graph_->GetEdge(4)->GetTrgWords());
        HyperEdge * e_12_y = new HyperEdge(n_12_y); exp_graph->AddEdge(e_12_y); n_12_y->AddEdge(e_12_y);
        e_12_y->SetFeatures(rule_graph_->GetEdge(5)->GetFeatures());
        e_12_y->AddFeature(Dict::WID("lm"), -0.8129134); // P(y)
        e_12_y->SetScore(-0.8129134 + -0.5);
        e_12_y->SetTrgWords(rule_graph_->GetEdge(5)->GetTrgWords());
        HyperEdge * e_12_t = new HyperEdge(n_12_t); exp_graph->AddEdge(e_12_t); n_12_t->AddEdge(e_12_t);
        e_12_t->SetFeatures(rule_graph_->GetEdge(6)->GetFeatures());
        e_12_t->AddFeature(Dict::WID("lm"), -100); // P(unk)
        e_12_t->SetScore(-100 + -2.5);
        e_12_t->SetTrgWords(rule_graph_->GetEdge(6)->GetTrgWords());
        // Make edges for 0,2. There are more than three, so only expand the best three
        HyperEdge * e_02_abx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_abx); n_02_ax->AddEdge(e_02_abx);
        e_02_abx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
        e_02_abx->AddFeature(Dict::WID("lm"), -0.4855544 - -0.8129134);
        e_02_abx->SetScore(-0.4855544 - -0.8129134 + -0.3);
        e_02_abx->AddTail(n_01_ab); e_02_abx->AddTail(n_12_x);
        e_02_abx->SetTrgWords(rule_graph_->GetEdge(0)->GetTrgWords());
        HyperEdge * e_02_acx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_acx); n_02_ax->AddEdge(e_02_acx);
        e_02_acx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
        e_02_acx->AddFeature(Dict::WID("lm"), -0.30103);
        e_02_acx->SetScore(-0.30103 + -0.3);
        e_02_acx->AddTail(n_01_ac); e_02_acx->AddTail(n_12_x);
        e_02_acx->SetTrgWords(rule_graph_->GetEdge(0)->GetTrgWords());
        HyperEdge * e_02_aby = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_aby); n_02_ay->AddEdge(e_02_aby);
        e_02_aby->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
        e_02_aby->AddFeature(Dict::WID("lm"), -0.30103);
        e_02_aby->SetScore(-0.30103 + -0.3);
        e_02_aby->AddTail(n_01_ab); e_02_aby->AddTail(n_12_y);
        e_02_aby->SetTrgWords(rule_graph_->GetEdge(0)->GetTrgWords());
        // Make edges for the root. There are only two
        HyperEdge * e_root_ax = new HyperEdge(n_root); exp_graph->AddEdge(e_root_ax); n_root->AddEdge(e_root_ax);
        e_root_ax->AddFeature(Dict::WID("lm"), -0.4372497 - -0.6368221 + -0.2108534);
        e_root_ax->SetScore(-0.4372497 - -0.6368221 + -0.2108534);
        e_root_ax->AddTail(n_02_ax);
        e_root_ax->AddTrgWord(-1);
        HyperEdge * e_root_ay = new HyperEdge(n_root); exp_graph->AddEdge(e_root_ay); n_root->AddEdge(e_root_ay);
        e_root_ay->AddFeature(Dict::WID("lm"), -0.4372497 - -0.6368221 + -0.30103 + -0.6368221);
        e_root_ay->SetScore(-0.4372497 - -0.6368221 + -0.30103 + -0.6368221);
        e_root_ay->AddTail(n_02_ay);
        e_root_ay->AddTrgWord(-1);
        return exp_graph->CheckEqual(*act_graph);
    }

    int TestUnaryFlatten() {
        UnaryFlattener flat;
        istringstream iss("(A (B s))");
        boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
        boost::scoped_ptr<HyperGraph> act_graph(flat.TransformGraph(*un_graph));
        ostringstream oss;
        tree_io_.WriteTree(*act_graph, oss);
        string exp_str = "(A_B s)", act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }

    int TestUnaryFlatten2() {
        UnaryFlattener flat;
        istringstream iss("(A s)");
        boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
        boost::scoped_ptr<HyperGraph> act_graph(flat.TransformGraph(*un_graph));
        ostringstream oss;
        tree_io_.WriteTree(*act_graph, oss);
        string exp_str = "(A s)", act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }

    int TestWordSplit() {
        WordSplitter splitter;
        istringstream iss("(A x-y)");
        boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
        boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
        ostringstream oss;
        tree_io_.WriteTree(*act_graph, oss);
        string exp_str = "(A (A x) (A -) (A y))", act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }
    int TestWordSplitConnected() {
        WordSplitter splitter;
        istringstream iss("(A x--y)");
        boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
        boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
        ostringstream oss;
        tree_io_.WriteTree(*act_graph, oss);
        string exp_str = "(A (A x) (A -) (A -) (A y))", act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }
    int TestWordSplitInitFinal() {
        WordSplitter splitter;
        istringstream iss("(A -x-)");
        boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
        boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
        ostringstream oss;
        tree_io_.WriteTree(*act_graph, oss);
        string exp_str = "(A (A -) (A x) (A -))", act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }
    int TestWordSplitSingle() {
        WordSplitter splitter("(a|b)");
        istringstream iss("(A a)");
        boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
        boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
        ostringstream oss;
        tree_io_.WriteTree(*act_graph, oss);
        string exp_str = "(A a)", act_str = oss.str();
        return CheckEqual(exp_str, act_str);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestLMIntersection()" << endl; if(TestLMIntersection()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestUnaryFlatten()" << endl; if(TestUnaryFlatten()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestUnaryFlatten2()" << endl; if(TestUnaryFlatten2()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWordSplit()" << endl; if(TestWordSplit()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWordSplitConnected()" << endl; if(TestWordSplitConnected()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWordSplitInitFinal()" << endl; if(TestWordSplitInitFinal()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestWordSplitSingle()" << endl; if(TestWordSplitSingle()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestGraphTransformer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    PennTreeIO tree_io_;
    JSONTreeIO json_tree_io_;
    std::vector<WordId> src_;
    boost::scoped_ptr<HyperGraph> rule_graph_, unary_graph_;
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

}

#endif
