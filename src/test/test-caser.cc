#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/caser.h>
#include <travatar/check-equal.h>
#include <travatar/dict.h>
#include <travatar/tree-io.h>
#include <travatar/hyper-graph.h>

using namespace std;
using namespace boost;
using namespace travatar;

// ****** The fixture *******
struct TestCaser {

    TestCaser() {
        caser_.AddTrueValue("GrüßeN");
        caser_.AddTrueValue("This");
        caser_.AddTrueValue("that");
    }

    ~TestCaser() { }

    Caser caser_;

};

// ****** The tests *******
BOOST_FIXTURE_TEST_SUITE(caser, TestCaser)

BOOST_AUTO_TEST_CASE(TestWordToLower) {
    BOOST_CHECK(CheckEqual(string("grüßen"), caser_.ToLower("grÜßEN")));
}

BOOST_AUTO_TEST_CASE(TestWordToTitle) {
    std::string grussen = "grüßEN";
    BOOST_CHECK(CheckEqual(string("Grüßen"), caser_.ToTitle("grÜßEN")));
}

BOOST_AUTO_TEST_CASE(TestWordTrueCase) {
    BOOST_CHECK(CheckEqual(string("GrüßeN"), caser_.TrueCase("grÜßEN")));
}

BOOST_AUTO_TEST_CASE(TestSentenceFirst) {
    Sentence val = Dict::ParseWords("Here are examples : sentence 1 . sentence 2 ? and sentence 3 ! yes");
    vector<bool> exp_first(15, false);
    exp_first[0] = true;
    exp_first[4] = true;
    exp_first[7] = true;
    exp_first[10] = true;
    exp_first[14] = true;
    vector<bool> act_first = caser_.SentenceFirst(val);
    BOOST_CHECK(CheckVector(exp_first, act_first));
}

BOOST_AUTO_TEST_CASE(TestSentenceToLower) {
    Sentence exp_val = Dict::ParseWords("this is a test .");
    Sentence act_val = Dict::ParseWords("This is a test .");
    caser_.ToLower(act_val);
    BOOST_CHECK(CheckVector(exp_val, act_val));
}

BOOST_AUTO_TEST_CASE(TestSentenceToTitle) {
    Sentence exp_val1 = Dict::ParseWords("This is a test .");
    Sentence act_val1 = Dict::ParseWords("this is a test .");
    Sentence exp_val2 = Dict::ParseWords("This is a test . And this also !");
    Sentence act_val2 = Dict::ParseWords("this is a test . and this also !");
    caser_.ToTitle(act_val1);
    caser_.ToTitle(act_val2);
    BOOST_CHECK(CheckVector(exp_val1, act_val1) && CheckVector(exp_val2, act_val2));
}

BOOST_AUTO_TEST_CASE(TestSentenceTrueCase) {
    Sentence exp_val1 = Dict::ParseWords("This is a test .");
    Sentence act_val1 = Dict::ParseWords("thiS is a test .");
    caser_.TrueCase(act_val1);
    Sentence exp_val2 = Dict::ParseWords("that is a test .");
    Sentence act_val2 = Dict::ParseWords("thaT is a test .");
    caser_.TrueCase(act_val2);
    Sentence exp_val3 = Dict::ParseWords("Black is a test .");
    Sentence act_val3 = Dict::ParseWords("Black is a test .");
    caser_.TrueCase(act_val3);
    BOOST_CHECK(CheckVector(exp_val1, act_val1));
    BOOST_CHECK(CheckVector(exp_val2, act_val2));
    BOOST_CHECK(CheckVector(exp_val3, act_val3));
}

BOOST_AUTO_TEST_CASE(TestHyperGraphToLower) {
    PennTreeIO io;
    string exp_str("(S (NP (PRP this)) (VP (VB is) (NP (DT a) (NN test))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val(io.ReadFromString(exp_str));
    string act_str("(S (NP (PRP This)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val(io.ReadFromString(act_str));
    caser_.ToLower(*act_val);
    BOOST_CHECK(exp_val->CheckEqual(*act_val));
}

BOOST_AUTO_TEST_CASE(TestHyperGraphToTitle) {
    PennTreeIO io;
    string exp_str("(S (NP (PRP This)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val(io.ReadFromString(exp_str));
    string act_str("(S (NP (PRP this)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val(io.ReadFromString(act_str));
    caser_.ToTitle(*act_val);
    BOOST_CHECK(exp_val->CheckEqual(*act_val));
}

BOOST_AUTO_TEST_CASE(TestHyperGraphTrueCase) {
    PennTreeIO io;
    string exp_str1("(S (NP (PRP This)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val1(io.ReadFromString(exp_str1));
    string act_str1("(S (NP (PRP thiS)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val1(io.ReadFromString(act_str1));
    string exp_str2("(S (NP (PRP that)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val2(io.ReadFromString(exp_str2));
    string act_str2("(S (NP (PRP thaT)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val2(io.ReadFromString(act_str2));
    string exp_str3("(S (NP (PRP Black)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val3(io.ReadFromString(exp_str3));
    string act_str3("(S (NP (PRP Black)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val3(io.ReadFromString(act_str3));
    caser_.TrueCase(*act_val1);
    caser_.TrueCase(*act_val2);
    caser_.TrueCase(*act_val3);
    BOOST_CHECK(exp_val1->CheckEqual(*act_val1));
    BOOST_CHECK(exp_val2->CheckEqual(*act_val2));
    BOOST_CHECK(exp_val3->CheckEqual(*act_val3));
}


BOOST_AUTO_TEST_SUITE_END()
