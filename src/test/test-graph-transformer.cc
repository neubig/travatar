#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/check-equal.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/translation-rule.h>
#include <travatar/unary-flattener.h>
#include <travatar/word-splitter-regex.h>
#include <travatar/word-splitter-compound.h>
#include <travatar/tree-io.h>
#include <fstream>
#include <sstream>
#include <utility>

using namespace std;
using namespace travatar;

// ***** The fixture *****
struct TestGraphTransformer {

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
        e0->GetFeatures().Add(Dict::WID("toy_feature"), 1.5);
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

    PennTreeIO tree_io_;
    JSONTreeIO json_tree_io_;
    std::vector<WordId> src_;
    boost::scoped_ptr<HyperGraph> rule_graph_, unary_graph_;
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

// ****** The tests *******
BOOST_FIXTURE_TEST_SUITE(graph_transformer, TestGraphTransformer)

BOOST_AUTO_TEST_CASE(TestUnaryFlatten) {
    UnaryFlattener flat;
    std::istringstream iss("(A (B s))");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(flat.TransformGraph(*un_graph));
    ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(A_B s)", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestUnaryFlatten2) {
    UnaryFlattener flat;
    std::istringstream iss("(A s)");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(flat.TransformGraph(*un_graph));
    ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(A s)", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestWordSplit) {
    WordSplitterRegex splitter("(\\-+|\\\\/)");
    std::istringstream iss("(A x\\/y)");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
    ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(A (A x) (A \\/) (A y))", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestWordSplitConnected) {
    WordSplitterRegex splitter;
    std::istringstream iss("(A x--y)");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
    ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(A (A x) (A -) (A -) (A y))", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestWordSplitInitFinal) {
    WordSplitterRegex splitter;
    std::istringstream iss("(A -x-)");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
    ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(A (A -) (A x) (A -))", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestWordSplitSingle) {
    WordSplitterRegex splitter("(a|b)");
    std::istringstream iss("(A a)");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
    ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(A a)", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestCompoundWordSplit) {
    std::string file_name = "/tmp/test-compoundsplit.arpa";
    std::ofstream arpa_out(file_name.c_str());
    arpa_out << ""
"\\data\\\n"
"ngram 1=7\n"
"ngram 2=8\n"
"\n"
"\\1-grams:\n"
"-0.6368221	</s>\n"
"-99	<s>	-0.30103\n"
"-0.6368221	auto	-0.4771213\n"
"-0.6368221	fahrer	-0.30103\n"
"-0.6368221	autobahn	-0.30103\n"
"-0.8129134	autofahrer	-0.30103\n"
"-0.8129134	bahn	-0.30103\n"
"\n"
"\\2-grams:\n"
"-0.4372497	<s> auto\n"
"-0.4855544	<s> bahn\n"
"-0.1286666	auto fahrer\n"
"-0.1286666	auto autobahn\n"
"-0.4372497	fahrer </s>\n"
"-0.4855544	fahrer autofahrer\n"
"-0.2108534	autofahrer </s>\n"
"-0.2108534	bahn auto" 
"\n"
"\\end\\\n" << endl;
    arpa_out.close();

    std::string filler = "";
    WordSplitterCompound splitter(file_name,1.0,3,filler);
    std::istringstream iss("(s (nn autofahrer) (nn autobahn))");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
    std::ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(s (nn (nn auto) (nn fahrer)) (nn autobahn))", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_CASE(TestCompoundWordSplitFiller) {

    std::string file_name = "/tmp/test-compoundsplit2.arpa";
    std::ofstream arpa_out(file_name.c_str());
    arpa_out << ""
"\\data\\\n"
"ngram 1=7\n"
"ngram 2=8\n"
"\n"
"\\1-grams:\n"
"-0.6368221	</s>\n"
"-99	<s>	-0.30103\n"
"-0.6368221	jahr	-0.4771213\n"
"-0.6368221	zeit	-0.30103\n"
"-0.6368221	promotion	-0.30103\n"
"-0.8129134	jahreszeit	-0.30103\n"
"-0.8129134	promotionzeit	-0.30103\n"
"\n"
"\\2-grams:\n"
"-0.4372497	<s> jahr\n"
"-0.4855544	<s> promotionzeit\n"
"-0.1286666	jahr zeit\n"
"-0.1286666	jahr promotion\n"
"-0.4372497	zeit </s>\n"
"-0.4855544	zeit jahreszeit\n"
"-0.2108534	jahreszeit </s>\n"
"-0.2108534	promotionzeit jahr" 
"\n"
"\\end\\\n" << endl;
    arpa_out.close();

    std::string filler = "s:es";
    WordSplitterCompound splitter(file_name,1.0,3,filler);
    std::istringstream iss("(s (nn promotionszeit) (nn jahreszeit))");
    boost::scoped_ptr<HyperGraph> un_graph(tree_io_.ReadTree(iss));
    boost::scoped_ptr<HyperGraph> act_graph(splitter.TransformGraph(*un_graph));
    std::ostringstream oss;
    tree_io_.WriteTree(*act_graph, oss);
    std::string exp_str = "(s (nn (nn promotion) (nn zeit)) (nn (nn jahr) (nn zeit)))", act_str = oss.str();
    BOOST_CHECK(CheckEqual(exp_str, act_str));
}

BOOST_AUTO_TEST_SUITE_END()
