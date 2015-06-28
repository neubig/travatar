#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/dict.h>
#include <travatar/trimmer-nbest.h>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

using namespace std;
using namespace boost;
using namespace travatar;

struct TestTrimmer {

    TestTrimmer() {
        // Example rule graph
        rule_graph_.reset(new HyperGraph);
        {
            vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
            rule_graph_->SetWords(ab);
            HyperNode * n0 = new HyperNode; n0->SetSpan(make_pair(0,2)); rule_graph_->AddNode(n0);
            HyperNode * n1 = new HyperNode; n1->SetSpan(make_pair(0,1)); rule_graph_->AddNode(n1);
            HyperNode * n2 = new HyperNode; n2->SetSpan(make_pair(1,2)); rule_graph_->AddNode(n2);
            rule_01.reset(new TranslationRule); rule_01->AddTrgWord(-1); rule_01->AddTrgWord(-2);
            HyperEdge * e0 = new HyperEdge(n0); rule_graph_->AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); e0->SetRule(rule_01.get()); n0->AddEdge(e0);
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
            rule_graph_->GetNode(0)->CalcViterbiScore();
        }
        // Two option graph
        binary_graph_.reset(new HyperGraph);
        {
            HyperNode * n0 = new HyperNode; n0->SetSpan(make_pair(0,1)); binary_graph_->AddNode(n0);
            HyperNode * n1 = new HyperNode; n1->SetSpan(make_pair(0,1)); binary_graph_->AddNode(n1);
            HyperNode * n2 = new HyperNode; n2->SetSpan(make_pair(0,1)); binary_graph_->AddNode(n2);
            HyperEdge * e0 = new HyperEdge(n0); binary_graph_->AddEdge(e0); e0->AddTail(n1); e0->SetScore(-0.5); n0->AddEdge(e0);
            HyperEdge * e1 = new HyperEdge(n0); binary_graph_->AddEdge(e1); e1->AddTail(n2); e1->SetScore(-0.3); n0->AddEdge(e1);
            HyperEdge * e2 = new HyperEdge(n1); binary_graph_->AddEdge(e2); e2->SetScore(-0.2); n1->AddEdge(e2);
            HyperEdge * e3 = new HyperEdge(n2); binary_graph_->AddEdge(e3); e3->SetScore(-0.3); n2->AddEdge(e3);
            binary_graph_->GetNode(0)->CalcViterbiScore();
        }
    }
    ~TestTrimmer() { }

    boost::shared_ptr<HyperGraph> rule_graph_, binary_graph_; 
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

// ****** The tests *******
BOOST_FIXTURE_TEST_SUITE(trimmer, TestTrimmer)

BOOST_AUTO_TEST_CASE(TestNbest) {
    // The expected rule graph for n-best of size 1
    // Only edges 0, 2, and 4 should remain
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    vector<int> ab(2); ab[0] = Dict::WID("s"); ab[1] = Dict::WID("t");
    exp_graph->SetWords(ab);
    HyperNode * n0 = new HyperNode; n0->SetSpan(make_pair(0,2)); exp_graph->AddNode(n0); n0->SetViterbiScore(-0.6);
    HyperNode * n1 = new HyperNode; n1->SetSpan(make_pair(0,1)); exp_graph->AddNode(n1); n1->SetViterbiScore(-0.1);
    HyperNode * n2 = new HyperNode; n2->SetSpan(make_pair(1,2)); exp_graph->AddNode(n2); n2->SetViterbiScore(-0.2);
    HyperEdge * e0 = new HyperEdge(n0); exp_graph->AddEdge(e0); e0->AddTail(n1); e0->AddTail(n2); e0->SetScore(-0.3); e0->SetRule(rule_01.get()); n0->AddEdge(e0);
    HyperEdge * e2 = new HyperEdge(n1); exp_graph->AddEdge(e2); e2->SetScore(-0.1); e2->SetRule(rule_a.get()); n1->AddEdge(e2);
    HyperEdge * e4 = new HyperEdge(n2); exp_graph->AddEdge(e4); e4->SetScore(-0.2); e4->SetRule(rule_x.get()); n2->AddEdge(e4);
    // Do the actual processing
    TrimmerNbest trim(1);
    boost::shared_ptr<HyperGraph> act_graph(trim.TransformGraph(*rule_graph_));
    BOOST_CHECK(exp_graph->CheckEqual(*act_graph));
}

BOOST_AUTO_TEST_CASE(TestNbestNode) {
    // The expected rule graph for n-best of size 1
    // Only edges 1 and nodes 0, 2 should remain
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    // Two option graph
    {
        HyperNode * n0 = new HyperNode; n0->SetSpan(make_pair(0,1)); exp_graph->AddNode(n0); n0->SetViterbiScore(-0.6);
        HyperNode * n2 = new HyperNode; n2->SetSpan(make_pair(0,1)); exp_graph->AddNode(n2); n2->SetViterbiScore(-0.3);
        HyperEdge * e1 = new HyperEdge(n0); exp_graph->AddEdge(e1); e1->AddTail(n2); e1->SetScore(-0.3); n0->AddEdge(e1);
        HyperEdge * e3 = new HyperEdge(n2); exp_graph->AddEdge(e3); e3->SetScore(-0.3); n2->AddEdge(e3);
    }
    // Do the actual processing
    TrimmerNbest trim(1);
    boost::shared_ptr<HyperGraph> act_graph(trim.TransformGraph(*binary_graph_));
    BOOST_CHECK(exp_graph->CheckEqual(*act_graph));
}


BOOST_AUTO_TEST_SUITE_END()
