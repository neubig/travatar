#include "test-lm-composer.h"
#include <travatar/lm-composer-incremental.h>
#include <travatar/lm-composer-bu.h>
#include <fstream>
#include <utility>

using namespace std;
using namespace boost;

namespace travatar {

TestLMComposer::TestLMComposer() {
    {
        // Example rule graph
        rule_graph_.reset(new HyperGraph);
        vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
        rule_graph_->SetWords(ab);
        HyperNode * n0 = new HyperNode; n0->SetSpan(make_pair(0,2)); rule_graph_->AddNode(n0);
        HyperNode * n1 = new HyperNode; n1->SetSpan(make_pair(0,1)); rule_graph_->AddNode(n1);
        HyperNode * n2 = new HyperNode; n2->SetSpan(make_pair(1,2)); rule_graph_->AddNode(n2);
        HyperNode * n3 = new HyperNode; n3->SetSpan(make_pair(1,2)); rule_graph_->AddNode(n3);
        // Rules for n0
        rule_01.reset(new TranslationRule); rule_01->AddTrgWord(-1); rule_01->AddTrgWord(-2);
        HyperEdge * e0 = new HyperEdge(n0); rule_graph_->AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); e0->SetRule(rule_01.get()); n0->AddEdge(e0);
        e0->AddFeature(Dict::WID("toy_feature"), 1.5);
        rule_10.reset(new TranslationRule); rule_10->AddTrgWord(-2); rule_10->AddTrgWord(-1);
        HyperEdge * e1 = new HyperEdge(n0); rule_graph_->AddEdge(e1); e1->AddTail(n1); e1->AddTail(n2); e1->SetScore(-0.7); e1->SetRule(rule_10.get()); n0->AddEdge(e1);
        // Rules for n1
        rule_a.reset(new TranslationRule); rule_a->AddTrgWord(Dict::WID("a")); rule_a->AddTrgWord(Dict::WID("b"));
        HyperEdge * e2 = new HyperEdge(n1); rule_graph_->AddEdge(e2); e2->SetScore(-0.1); e2->SetRule(rule_a.get()); n1->AddEdge(e2);
        rule_b.reset(new TranslationRule); rule_b->AddTrgWord(Dict::WID("a")); rule_b->AddTrgWord(Dict::WID("c"));
        HyperEdge * e3 = new HyperEdge(n1); rule_graph_->AddEdge(e3); e3->SetScore(-0.3); e3->SetRule(rule_b.get()); n1->AddEdge(e3);
        // Rules for n2
        rule_x.reset(new TranslationRule); rule_x->AddTrgWord(Dict::WID("x"));
        HyperEdge * e4 = new HyperEdge(n2); rule_graph_->AddEdge(e4); e4->SetScore(-0.2); e4->SetRule(rule_x.get()); n2->AddEdge(e4);
        rule_y.reset(new TranslationRule); rule_y->AddTrgWord(Dict::WID("y"));
        HyperEdge * e5 = new HyperEdge(n2); rule_graph_->AddEdge(e5); e5->SetScore(-0.5); e5->SetRule(rule_y.get()); n2->AddEdge(e5);
        rule_unk.reset(new TranslationRule); rule_unk->AddTrgWord(Dict::WID("<unk>"));
        HyperEdge * e6 = new HyperEdge(n2); rule_graph_->AddEdge(e6); e6->SetScore(-2.5); e6->SetRule(rule_unk.get()); n2->AddEdge(e6);
        // A rule with an empty node. This should just be ignored.
        rule_01bad.reset(new TranslationRule); rule_01bad->AddTrgWord(-1); rule_01bad->AddTrgWord(-2);
        HyperEdge * e7 = new HyperEdge(n0); rule_graph_->AddEdge(e7); e7->AddTail(n1); e7->AddTail(n3); e7->SetScore(-0.3); e7->SetRule(rule_01bad.get()); n0->AddEdge(e7);
    }

    // Create the n-gram model
    file_name_ = "/tmp/test-hyper-graph-lm.arpa";
    ofstream arpa_out(file_name_.c_str());
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
"-0.7368221	c	-0.30103\n"
"-0.8129134	x	-0.30103\n"
"-0.8129134	y	-0.30103\n"
"\n"
"\\2-grams:\n"
"-0.4372497	<s> a\n"
"-0.4855544	<s> y\n"
"-0.1286666	a b\n"
"-0.1786666	a c\n"
"-0.4372497	b </s>\n"
"-0.4855544	b x\n"
"-0.2108534	x </s>\n"
"-0.2108534	y a" 
"\n"
"\\end\\\n" << endl;
    arpa_out.close();
}

TestLMComposer::~TestLMComposer() { }

int TestLMComposer::TestLMComposerBU() {
    // Create the expected graph
    vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
    exp_graph.reset(new HyperGraph);
    exp_graph->SetWords(ab);
    // The root node should be "0,2"
    HyperNode * n_root = new HyperNode; n_root->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_root);
    n_root->SetSym(Dict::WID("LMROOT"));
    HyperNode * n_01_ab = new HyperNode; n_01_ab->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ab);
    n_01_ab->SetViterbiScore(-0.1286666 + -0.6368221 + -0.1);
    HyperNode * n_01_ac = new HyperNode; n_01_ac->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ac);
    n_01_ac->SetViterbiScore(-0.1786666 + -0.6368221 + -0.3);
    // Options on the right node should be "x" and "y"
    HyperNode * n_12_x = new HyperNode; n_12_x->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_x);
    n_12_x->SetViterbiScore(-0.8129134 + -0.2);
    HyperNode * n_12_y = new HyperNode; n_12_y->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_y);
    n_12_y->SetViterbiScore(-0.8129134 + -0.5);
    HyperNode * n_12_t = new HyperNode; n_12_t->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_t);
    n_12_t->SetViterbiScore(-100 + -2.5 + -20);
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
    e_01_ab->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(2)->GetTrgData()[0].words))));
    HyperEdge * e_01_ac = new HyperEdge(n_01_ac); exp_graph->AddEdge(e_01_ac); n_01_ac->AddEdge(e_01_ac);
    e_01_ac->SetFeatures(rule_graph_->GetEdge(3)->GetFeatures());
    e_01_ac->AddFeature(Dict::WID("lm"), -0.1786666 + -0.6368221);
    e_01_ac->SetScore(-0.1786666 + -0.6368221 + -0.3);
    e_01_ac->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(3)->GetTrgData()[0].words))));
    // Make edges for 1,2. There are only 3, so no pruning
    HyperEdge * e_12_x = new HyperEdge(n_12_x); exp_graph->AddEdge(e_12_x); n_12_x->AddEdge(e_12_x);
    e_12_x->SetFeatures(rule_graph_->GetEdge(4)->GetFeatures());
    e_12_x->AddFeature(Dict::WID("lm"), -0.8129134); // P(x)
    e_12_x->SetScore(-0.8129134 + -0.2);
    e_12_x->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(4)->GetTrgData()[0].words))));
    HyperEdge * e_12_y = new HyperEdge(n_12_y); exp_graph->AddEdge(e_12_y); n_12_y->AddEdge(e_12_y);
    e_12_y->SetFeatures(rule_graph_->GetEdge(5)->GetFeatures());
    e_12_y->AddFeature(Dict::WID("lm"), -0.8129134); // P(y)
    e_12_y->SetScore(-0.8129134 + -0.5);
    e_12_y->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(5)->GetTrgData()[0].words))));
    HyperEdge * e_12_t = new HyperEdge(n_12_t); exp_graph->AddEdge(e_12_t); n_12_t->AddEdge(e_12_t);
    e_12_t->SetFeatures(rule_graph_->GetEdge(6)->GetFeatures());
    e_12_t->AddFeature(Dict::WID("lm"), -100); // P(unk)
    e_12_t->AddFeature(Dict::WID("lmunk"), 1); // P(unk)
    e_12_t->SetScore(-100 + -2.5 + -20);
    e_12_t->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(6)->GetTrgData()[0].words))));
    // Make edges for 0,2. There are more than three, so only expand the best three
    HyperEdge * e_02_abx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_abx); n_02_ax->AddEdge(e_02_abx);
    e_02_abx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_abx->AddFeature(Dict::WID("lm"), -0.4855544 - -0.8129134);
    e_02_abx->SetScore(-0.4855544 - -0.8129134 + -0.3);
    e_02_abx->AddTail(n_01_ab); e_02_abx->AddTail(n_12_x);
    e_02_abx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_acx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_acx); n_02_ax->AddEdge(e_02_acx);
    e_02_acx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_acx->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_acx->SetScore(-0.30103 + -0.3);
    e_02_acx->AddTail(n_01_ac); e_02_acx->AddTail(n_12_x);
    e_02_acx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_aby = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_aby); n_02_ay->AddEdge(e_02_aby);
    e_02_aby->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_aby->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_aby->SetScore(-0.30103 + -0.3);
    e_02_aby->AddTail(n_01_ab); e_02_aby->AddTail(n_12_y);
    e_02_aby->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
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

    // Intersect the graph with the LM
    LMComposerBU lm(file_name_.c_str());
    lm.SetStackPopLimit(3);
    SparseMap weights;
    weights[Dict::WID("lmunk")] = -20;
    weights[Dict::WID("lm")] = 1;
    lm.UpdateWeights(weights);
    shared_ptr<HyperGraph> act_graph(lm.TransformGraph(*rule_graph_));
    return exp_graph->CheckEqual(*act_graph);
}

int TestLMComposer::TestLMComposerIncremental() {
    // Create the expected graph
    vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
    exp_graph.reset(new HyperGraph);
    exp_graph->SetWords(ab);
    // The root node should be "0,2"
    HyperNode * n_root = new HyperNode; n_root->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_root);
    n_root->SetSym(Dict::WID("LMROOT"));
    HyperNode * n_01_ac = new HyperNode; n_01_ac->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ac);
    n_01_ac->SetViterbiScore(-0.1786666 + -0.6368221 + -0.3);
    HyperNode * n_01_ab = new HyperNode; n_01_ab->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ab);
    n_01_ab->SetViterbiScore(-0.1286666 + -0.6368221 + -0.1);
    // Options on the right node should be "x" and "y"
    HyperNode * n_12_t = new HyperNode; n_12_t->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_t);
    n_12_t->SetViterbiScore(-100 + -2.5 + -20);
    HyperNode * n_12_y = new HyperNode; n_12_y->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_y);
    n_12_y->SetViterbiScore(-0.8129134 + -0.5);
    HyperNode * n_12_x = new HyperNode; n_12_x->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_x);
    n_12_x->SetViterbiScore(-0.8129134 + -0.2);
    // Options on the top node include a*x, a*y, x*b, x*c, y*b, y*c
    HyperNode * n_02_ay = new HyperNode; n_02_ay->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ay);
    n_02_ay->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.3 + -0.30103);
    HyperNode * n_02_ax = new HyperNode; n_02_ax->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ax);
    n_02_ax->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_x->GetViterbiScore() + -0.3 + -0.4855544 - -0.8129134);
    n_root->SetViterbiScore(n_02_ax->GetViterbiScore() + -0.4372497 - -0.6368221 + -0.2108534);
    // Make edges for 0,1. There are only 2, so no pruning
    HyperEdge * e_01_ac = new HyperEdge(n_01_ac); exp_graph->AddEdge(e_01_ac); n_01_ac->AddEdge(e_01_ac);
    e_01_ac->SetFeatures(rule_graph_->GetEdge(3)->GetFeatures());
    e_01_ac->AddFeature(Dict::WID("lm"), -0.1786666 + -0.6368221);
    e_01_ac->SetScore(-0.1786666 + -0.6368221 + -0.3);
    e_01_ac->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(3)->GetTrgData()[0].words))));
    HyperEdge * e_01_ab = new HyperEdge(n_01_ab); exp_graph->AddEdge(e_01_ab); n_01_ab->AddEdge(e_01_ab);
    e_01_ab->SetFeatures(rule_graph_->GetEdge(2)->GetFeatures());
    e_01_ab->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
    e_01_ab->SetScore(-0.1286666 + -0.6368221 + -0.1);
    e_01_ab->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(2)->GetTrgData()[0].words))));
    // Make edges for 1,2. There are only 3, so no pruning
    HyperEdge * e_12_t = new HyperEdge(n_12_t); exp_graph->AddEdge(e_12_t); n_12_t->AddEdge(e_12_t);
    e_12_t->SetFeatures(rule_graph_->GetEdge(6)->GetFeatures());
    e_12_t->AddFeature(Dict::WID("lm"), -100); // P(unk)
    e_12_t->AddFeature(Dict::WID("lmunk"), 1); // P(unk)
    e_12_t->SetScore(-100 + -2.5 + -20);
    e_12_t->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(6)->GetTrgData()[0].words))));
    HyperEdge * e_12_y = new HyperEdge(n_12_y); exp_graph->AddEdge(e_12_y); n_12_y->AddEdge(e_12_y);
    e_12_y->SetFeatures(rule_graph_->GetEdge(5)->GetFeatures());
    e_12_y->AddFeature(Dict::WID("lm"), -0.8129134); // P(y)
    e_12_y->SetScore(-0.8129134 + -0.5);
    e_12_y->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(5)->GetTrgData()[0].words))));
    HyperEdge * e_12_x = new HyperEdge(n_12_x); exp_graph->AddEdge(e_12_x); n_12_x->AddEdge(e_12_x);
    e_12_x->SetFeatures(rule_graph_->GetEdge(4)->GetFeatures());
    e_12_x->AddFeature(Dict::WID("lm"), -0.8129134); // P(x)
    e_12_x->SetScore(-0.8129134 + -0.2);
    e_12_x->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(4)->GetTrgData()[0].words))));
    // Make edges for 0,2. There are more than three, so only expand the best three
    HyperEdge * e_02_aby = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_aby); n_02_ay->AddEdge(e_02_aby);
    e_02_aby->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_aby->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_aby->SetScore(-0.30103 + -0.3);
    e_02_aby->AddTail(n_01_ab); e_02_aby->AddTail(n_12_y);
    e_02_aby->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_abx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_abx); n_02_ax->AddEdge(e_02_abx);
    e_02_abx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_abx->AddFeature(Dict::WID("lm"), -0.4855544 - -0.8129134);
    e_02_abx->SetScore(-0.4855544 - -0.8129134 + -0.3);
    e_02_abx->AddTail(n_01_ab); e_02_abx->AddTail(n_12_x);
    e_02_abx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_acx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_acx); n_02_ax->AddEdge(e_02_acx);
    e_02_acx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_acx->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_acx->SetScore(-0.30103 + -0.3);
    e_02_acx->AddTail(n_01_ac); e_02_acx->AddTail(n_12_x);
    e_02_acx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
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

    // Intersect the graph with the LM
    LMComposerIncremental lm(file_name_);
    lm.SetStackPopLimit(3);
    SparseMap weights;
    weights[Dict::WID("lmunk")] = -20;
    weights[Dict::WID("lm")] = 1;
    lm.UpdateWeights(weights);
    shared_ptr<HyperGraph> act_graph(lm.TransformGraph(*rule_graph_));
    return act_graph.get() && exp_graph->CheckMaybeEqual(*act_graph);
}

int TestLMComposer::TestReverseBU() {
    // Create the expected graph
    vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
    exp_graph.reset(new HyperGraph);
    exp_graph->SetWords(ab);
    // The root node should be "0,2"
    HyperNode * n_root = new HyperNode; n_root->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_root);
    n_root->SetSym(Dict::WID("LMROOT"));
    HyperNode * n_01_ab = new HyperNode; n_01_ab->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ab);
    n_01_ab->SetViterbiScore(-0.1286666 + -0.6368221 + -0.1);
    HyperNode * n_01_ac = new HyperNode; n_01_ac->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ac);
    n_01_ac->SetViterbiScore(-0.1786666 + -0.6368221 + -0.3);
    // Options on the right node should be "x" and "y"
    HyperNode * n_12_x = new HyperNode; n_12_x->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_x);
    n_12_x->SetViterbiScore(-0.8129134 + -0.2);
    HyperNode * n_12_y = new HyperNode; n_12_y->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_y);
    n_12_y->SetViterbiScore(-0.8129134 + -0.5);
    HyperNode * n_12_t = new HyperNode; n_12_t->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_t);
    n_12_t->SetViterbiScore(-100 + -2.5 + -20);
    // Options on the top node include a*x, a*y, x*b, x*c, y*b, y*c
    HyperNode * n_02_ax = new HyperNode; n_02_ax->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ax);
    n_02_ax->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_x->GetViterbiScore() + -0.3 + -0.4855544 - -0.8129134);
    HyperNode * n_02_ay = new HyperNode; n_02_ay->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ay);
    n_02_ay->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.3 + -0.30103);
    HyperNode * n_02_xb = new HyperNode; n_02_xb->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_xb);
    n_02_xb->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_x->GetViterbiScore() + -0.7 + -0.30103);
    n_root->SetViterbiScore(n_02_ax->GetViterbiScore() + -0.4372497 - -0.6368221 + -0.2108534);
    // Make edges for 0,1. There are only 2, so no pruning
    HyperEdge * e_01_ab = new HyperEdge(n_01_ab); exp_graph->AddEdge(e_01_ab); n_01_ab->AddEdge(e_01_ab);
    e_01_ab->SetFeatures(rule_graph_->GetEdge(2)->GetFeatures());
    e_01_ab->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
    e_01_ab->SetScore(-0.1286666 + -0.6368221 + -0.1);
    e_01_ab->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(2)->GetTrgData()[0].words))));
    HyperEdge * e_01_ac = new HyperEdge(n_01_ac); exp_graph->AddEdge(e_01_ac); n_01_ac->AddEdge(e_01_ac);
    e_01_ac->SetFeatures(rule_graph_->GetEdge(3)->GetFeatures());
    e_01_ac->AddFeature(Dict::WID("lm"), -0.1786666 + -0.6368221);
    e_01_ac->SetScore(-0.1786666 + -0.6368221 + -0.3);
    e_01_ac->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(3)->GetTrgData()[0].words))));
    // Make edges for 1,2. There are only 3, so no pruning
    HyperEdge * e_12_x = new HyperEdge(n_12_x); exp_graph->AddEdge(e_12_x); n_12_x->AddEdge(e_12_x);
    e_12_x->SetFeatures(rule_graph_->GetEdge(4)->GetFeatures());
    e_12_x->AddFeature(Dict::WID("lm"), -0.8129134); // P(x)
    e_12_x->SetScore(-0.8129134 + -0.2);
    e_12_x->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(4)->GetTrgData()[0].words))));
    HyperEdge * e_12_y = new HyperEdge(n_12_y); exp_graph->AddEdge(e_12_y); n_12_y->AddEdge(e_12_y);
    e_12_y->SetFeatures(rule_graph_->GetEdge(5)->GetFeatures());
    e_12_y->AddFeature(Dict::WID("lm"), -0.8129134); // P(y)
    e_12_y->SetScore(-0.8129134 + -0.5);
    e_12_y->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(5)->GetTrgData()[0].words))));
    HyperEdge * e_12_t = new HyperEdge(n_12_t); exp_graph->AddEdge(e_12_t); n_12_t->AddEdge(e_12_t);
    e_12_t->SetFeatures(rule_graph_->GetEdge(6)->GetFeatures());
    e_12_t->AddFeature(Dict::WID("lm"), -100); // P(unk)
    e_12_t->AddFeature(Dict::WID("lmunk"), 1); // P(unk)
    e_12_t->SetScore(-100 + -2.5 + -20);
    e_12_t->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(6)->GetTrgData()[0].words))));
    // Make edges for 0,2. There are more than 5, so only expand the best 5
    HyperEdge * e_02_abx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_abx); n_02_ax->AddEdge(e_02_abx);
    e_02_abx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_abx->AddFeature(Dict::WID("lm"), -0.4855544 - -0.8129134);
    e_02_abx->SetScore(-0.4855544 - -0.8129134 + -0.3);
    e_02_abx->AddTail(n_01_ab); e_02_abx->AddTail(n_12_x);
    e_02_abx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_acx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_acx); n_02_ax->AddEdge(e_02_acx);
    e_02_acx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_acx->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_acx->SetScore(-0.30103 + -0.3);
    e_02_acx->AddTail(n_01_ac); e_02_acx->AddTail(n_12_x);
    e_02_acx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_aby = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_aby); n_02_ay->AddEdge(e_02_aby);
    e_02_aby->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_aby->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_aby->SetScore(-0.30103 + -0.3);
    e_02_aby->AddTail(n_01_ab); e_02_aby->AddTail(n_12_y);
    e_02_aby->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_acy = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_acy); n_02_ay->AddEdge(e_02_acy);
    e_02_acy->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_acy->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_acy->SetScore(-0.30103 + -0.3);
    e_02_acy->AddTail(n_01_ac); e_02_acy->AddTail(n_12_y);
    e_02_acy->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_xab = new HyperEdge(n_02_xb); exp_graph->AddEdge(e_02_xab); n_02_xb->AddEdge(e_02_xab);
    e_02_xab->SetFeatures(rule_graph_->GetEdge(1)->GetFeatures());
    e_02_xab->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_xab->SetScore(-0.30103 + -0.7);
    e_02_xab->AddTail(n_01_ab); e_02_xab->AddTail(n_12_x);
    e_02_xab->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(1)->GetTrgData()[0].words))));
    // Make edges for the root. There are three
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
    HyperEdge * e_root_xb = new HyperEdge(n_root); exp_graph->AddEdge(e_root_xb); n_root->AddEdge(e_root_xb);
    e_root_xb->AddFeature(Dict::WID("lm"), -0.30103 + -0.4372497);
    e_root_xb->SetScore(-0.30103 + -0.4372497);
    e_root_xb->AddTail(n_02_xb);
    e_root_xb->AddTrgWord(-1);
    // Intersect the graph with the LM
    LMComposerBU lm(file_name_.c_str());
    lm.SetStackPopLimit(5);
    SparseMap weights;
    weights[Dict::WID("lmunk")] = -20;
    weights[Dict::WID("lm")] = 1;
    lm.UpdateWeights(weights);
    shared_ptr<HyperGraph> act_graph(lm.TransformGraph(*rule_graph_));
    return act_graph.get() && exp_graph->CheckEqual(*act_graph);
}

int TestLMComposer::TestReverseIncremental() {
    // Create the expected graph
    vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
    exp_graph.reset(new HyperGraph);
    exp_graph->SetWords(ab);
    // The root node should be "0,2"
    HyperNode * n_root = new HyperNode; n_root->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_root);
    n_root->SetSym(Dict::WID("LMROOT"));
    HyperNode * n_01_ac = new HyperNode; n_01_ac->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ac);
    n_01_ac->SetViterbiScore(-0.1786666 + -0.6368221 + -0.3);
    HyperNode * n_01_ab = new HyperNode; n_01_ab->SetSpan(make_pair(0,1)); exp_graph->AddNode(n_01_ab);
    n_01_ab->SetViterbiScore(-0.1286666 + -0.6368221 + -0.1);
    // Options on the right node should be "x" and "y"
    HyperNode * n_12_t = new HyperNode; n_12_t->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_t);
    n_12_t->SetViterbiScore(-100 + -2.5 + -20);
    HyperNode * n_12_y = new HyperNode; n_12_y->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_y);
    n_12_y->SetViterbiScore(-0.8129134 + -0.5);
    HyperNode * n_12_x = new HyperNode; n_12_x->SetSpan(make_pair(1,2)); exp_graph->AddNode(n_12_x);
    n_12_x->SetViterbiScore(-0.8129134 + -0.2);
    // Options on the top node include a*x, a*y, x*b, x*c, y*b, y*c
    HyperNode * n_02_yc = new HyperNode; n_02_yc->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_yc);
    n_02_yc->SetViterbiScore(n_01_ac->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.2108534 - -0.6368221 + -0.7);
    HyperNode * n_02_yb = new HyperNode; n_02_yb->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_yb);
    n_02_yb->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.2108534 - -0.6368221 + -0.7);
    HyperNode * n_02_ay = new HyperNode; n_02_ay->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ay);
    n_02_ay->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_y->GetViterbiScore() + -0.3 + -0.30103);
    HyperNode * n_02_ax = new HyperNode; n_02_ax->SetSpan(make_pair(0,2)); exp_graph->AddNode(n_02_ax);
    n_02_ax->SetViterbiScore(n_01_ab->GetViterbiScore() + n_12_x->GetViterbiScore() + -0.3 + -0.4855544 - -0.8129134);
    n_root->SetViterbiScore(n_02_ax->GetViterbiScore() + -0.4372497 - -0.6368221 + -0.2108534);
    // Make edges for 0,1. There are only 2, so no pruning
    HyperEdge * e_01_ac = new HyperEdge(n_01_ac); exp_graph->AddEdge(e_01_ac); n_01_ac->AddEdge(e_01_ac);
    e_01_ac->SetFeatures(rule_graph_->GetEdge(3)->GetFeatures());
    e_01_ac->AddFeature(Dict::WID("lm"), -0.1786666 + -0.6368221);
    e_01_ac->SetScore(-0.1786666 + -0.6368221 + -0.3);
    e_01_ac->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(3)->GetTrgData()[0].words))));
    HyperEdge * e_01_ab = new HyperEdge(n_01_ab); exp_graph->AddEdge(e_01_ab); n_01_ab->AddEdge(e_01_ab);
    e_01_ab->SetFeatures(rule_graph_->GetEdge(2)->GetFeatures());
    e_01_ab->AddFeature(Dict::WID("lm"), -0.1286666 + -0.6368221);
    e_01_ab->SetScore(-0.1286666 + -0.6368221 + -0.1);
    e_01_ab->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(2)->GetTrgData()[0].words))));
    // Make edges for 1,2. There are only 3, so no pruning
    HyperEdge * e_12_t = new HyperEdge(n_12_t); exp_graph->AddEdge(e_12_t); n_12_t->AddEdge(e_12_t);
    e_12_t->SetFeatures(rule_graph_->GetEdge(6)->GetFeatures());
    e_12_t->AddFeature(Dict::WID("lm"), -100); // P(unk)
    e_12_t->AddFeature(Dict::WID("lmunk"), 1); // P(unk)
    e_12_t->SetScore(-100 + -2.5 + -20);
    e_12_t->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(6)->GetTrgData()[0].words))));
    HyperEdge * e_12_y = new HyperEdge(n_12_y); exp_graph->AddEdge(e_12_y); n_12_y->AddEdge(e_12_y);
    e_12_y->SetFeatures(rule_graph_->GetEdge(5)->GetFeatures());
    e_12_y->AddFeature(Dict::WID("lm"), -0.8129134); // P(y)
    e_12_y->SetScore(-0.8129134 + -0.5);
    e_12_y->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(5)->GetTrgData()[0].words))));
    HyperEdge * e_12_x = new HyperEdge(n_12_x); exp_graph->AddEdge(e_12_x); n_12_x->AddEdge(e_12_x);
    e_12_x->SetFeatures(rule_graph_->GetEdge(4)->GetFeatures());
    e_12_x->AddFeature(Dict::WID("lm"), -0.8129134); // P(x)
    e_12_x->SetScore(-0.8129134 + -0.2);
    e_12_x->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(4)->GetTrgData()[0].words))));
    // Make edges for 0,2. There are more than 5, so only expand the best 5
    HyperEdge * e_02_yac = new HyperEdge(n_02_yc); exp_graph->AddEdge(e_02_yac); n_02_yc->AddEdge(e_02_yac);
    e_02_yac->SetFeatures(rule_graph_->GetEdge(1)->GetFeatures());
    e_02_yac->AddFeature(Dict::WID("lm"), -0.2108534 - -0.6368221);
    e_02_yac->SetScore(-0.2108534 - -0.6368221 + -0.7);
    e_02_yac->AddTail(n_01_ac); e_02_yac->AddTail(n_12_y);
    e_02_yac->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(1)->GetTrgData()[0].words))));
    HyperEdge * e_02_yab = new HyperEdge(n_02_yb); exp_graph->AddEdge(e_02_yab); n_02_yb->AddEdge(e_02_yab);
    e_02_yab->SetFeatures(rule_graph_->GetEdge(1)->GetFeatures());
    e_02_yab->AddFeature(Dict::WID("lm"), -0.2108534 - -0.6368221);
    e_02_yab->SetScore(-0.2108534 - -0.6368221 + -0.7);
    e_02_yab->AddTail(n_01_ab); e_02_yab->AddTail(n_12_y);
    e_02_yab->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(1)->GetTrgData()[0].words))));
    HyperEdge * e_02_aby = new HyperEdge(n_02_ay); exp_graph->AddEdge(e_02_aby); n_02_ay->AddEdge(e_02_aby);
    e_02_aby->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_aby->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_aby->SetScore(-0.30103 + -0.3);
    e_02_aby->AddTail(n_01_ab); e_02_aby->AddTail(n_12_y);
    e_02_aby->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_abx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_abx); n_02_ax->AddEdge(e_02_abx);
    e_02_abx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_abx->AddFeature(Dict::WID("lm"), -0.4855544 - -0.8129134);
    e_02_abx->SetScore(-0.4855544 - -0.8129134 + -0.3);
    e_02_abx->AddTail(n_01_ab); e_02_abx->AddTail(n_12_x);
    e_02_abx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    HyperEdge * e_02_acx = new HyperEdge(n_02_ax); exp_graph->AddEdge(e_02_acx); n_02_ax->AddEdge(e_02_acx);
    e_02_acx->SetFeatures(rule_graph_->GetEdge(0)->GetFeatures());
    e_02_acx->AddFeature(Dict::WID("lm"), -0.30103);
    e_02_acx->SetScore(-0.30103 + -0.3);
    e_02_acx->AddTail(n_01_ac); e_02_acx->AddTail(n_12_x);
    e_02_acx->SetTrgData(CfgDataVector(1, CfgData((rule_graph_->GetEdge(0)->GetTrgData()[0].words))));
    // Make edges for the root. There are four
    HyperEdge * e_root_ax = new HyperEdge(n_root); exp_graph->AddEdge(e_root_ax); n_root->AddEdge(e_root_ax);
    e_root_ax->AddFeature(Dict::WID("lm"), -0.4372497 - -0.6368221 + -0.2108534);
    e_root_ax->SetScore(-0.4372497 - -0.6368221 + -0.2108534);
    e_root_ax->AddTail(n_02_ax);
    e_root_ax->AddTrgWord(-1);
    HyperEdge * e_root_yb = new HyperEdge(n_root); exp_graph->AddEdge(e_root_yb); n_root->AddEdge(e_root_yb);
    e_root_yb->AddFeature(Dict::WID("lm"), -0.4372497 + -0.4855544 - -0.8129134);
    e_root_yb->SetScore(-0.4372497 + -0.4855544 - -0.8129134);
    e_root_yb->AddTail(n_02_yb);
    e_root_yb->AddTrgWord(-1);
    HyperEdge * e_root_yc = new HyperEdge(n_root); exp_graph->AddEdge(e_root_yc); n_root->AddEdge(e_root_yc);
    e_root_yc->AddFeature(Dict::WID("lm"), -0.6368221 + -0.30103 + -0.4855544 - -0.8129134);
    e_root_yc->SetScore(-0.6368221 + -0.30103 + -0.4855544 - -0.8129134);
    e_root_yc->AddTail(n_02_yc);
    e_root_yc->AddTrgWord(-1);
    HyperEdge * e_root_ay = new HyperEdge(n_root); exp_graph->AddEdge(e_root_ay); n_root->AddEdge(e_root_ay);
    e_root_ay->AddFeature(Dict::WID("lm"), -0.4372497 - -0.6368221 + -0.30103 + -0.6368221);
    e_root_ay->SetScore(-0.4372497 - -0.6368221 + -0.30103 + -0.6368221);
    e_root_ay->AddTail(n_02_ay);
    e_root_ay->AddTrgWord(-1);

    // Intersect the graph with the LM
    LMComposerIncremental lm(file_name_.c_str());
    lm.SetStackPopLimit(5);
    SparseMap weights;
    weights[Dict::WID("lmunk")] = -20;
    weights[Dict::WID("lm")] = 1;
    lm.UpdateWeights(weights);
    shared_ptr<HyperGraph> act_graph(lm.TransformGraph(*rule_graph_));
    return act_graph.get() && exp_graph->CheckMaybeEqual(*act_graph);
}

bool TestLMComposer::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestLMComposerBU()" << endl; if(TestLMComposerBU()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestLMComposerIncremental()" << endl; if(TestLMComposerIncremental()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestReverseBU()" << endl; if(TestReverseBU()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestReverseIncremental()" << endl; if(TestReverseIncremental()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestLMComposer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

