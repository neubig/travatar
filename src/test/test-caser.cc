#include "test-caser.h"

#include <travatar/dict.h>
#include <travatar/check-equal.h>
#include <travatar/tree-io.h>
#include <travatar/hyper-graph.h>

#include <vector>

using namespace std;
using namespace boost;

namespace travatar {

TestCaser::TestCaser() {
    caser_.AddTrueValue("GrüßeN");
    caser_.AddTrueValue("This");
    caser_.AddTrueValue("that");
}
TestCaser::~TestCaser() { }

int TestCaser::TestWordToLower() {
    return CheckEqual(string("grüßen"), caser_.ToLower("grÜßEN"));
}

int TestCaser::TestWordToTitle() {
    std::string grussen = "grüßEN";
    return CheckEqual(string("Grüßen"), caser_.ToTitle("grÜßEN"));
}

int TestCaser::TestWordTrueCase() {
    return CheckEqual(string("GrüßeN"), caser_.TrueCase("grÜßEN"));
}

int TestCaser::TestSentenceFirst() {
    Sentence val = Dict::ParseWords("This is a test .");
    vector<bool> exp_first(5, false); exp_first[0] = true;
    vector<bool> act_first = caser_.SentenceFirst(val);
    return CheckVector(exp_first, act_first);
}

int TestCaser::TestSentenceToLower() {
    Sentence exp_val = Dict::ParseWords("this is a test .");
    Sentence act_val = Dict::ParseWords("This is a test .");
    caser_.ToLower(act_val);
    return CheckEqual(exp_val, act_val);
}

int TestCaser::TestSentenceToTitle() {
    Sentence exp_val = Dict::ParseWords("This is a test .");
    Sentence act_val = Dict::ParseWords("this is a test .");
    caser_.ToTitle(act_val);
    return CheckEqual(exp_val, act_val);
}

int TestCaser::TestSentenceTrueCase() {
    Sentence exp_val1 = Dict::ParseWords("This is a test .");
    Sentence act_val1 = Dict::ParseWords("thiS is a test .");
    caser_.TrueCase(act_val1);
    Sentence exp_val2 = Dict::ParseWords("that is a test .");
    Sentence act_val2 = Dict::ParseWords("thaT is a test .");
    caser_.TrueCase(act_val2);
    Sentence exp_val3 = Dict::ParseWords("Black is a test .");
    Sentence act_val3 = Dict::ParseWords("Black is a test .");
    caser_.TrueCase(act_val3);
    return 
        CheckEqual(exp_val1, act_val1) &&
        CheckEqual(exp_val2, act_val2) &&
        CheckEqual(exp_val3, act_val3);
}

int TestCaser::TestHyperGraphToLower() {
    PennTreeIO io;
    string exp_str("(S (NP (PRP this)) (VP (VB is) (NP (DT a) (NN test))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val(io.ReadFromString(exp_str));
    string act_str("(S (NP (PRP This)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val(io.ReadFromString(act_str));
    caser_.ToLower(*act_val);
    return exp_val->CheckEqual(*act_val);
}

int TestCaser::TestHyperGraphToTitle() {
    PennTreeIO io;
    string exp_str("(S (NP (PRP This)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> exp_val(io.ReadFromString(exp_str));
    string act_str("(S (NP (PRP this)) (VP (VB is) (NP (DT a) (NN TEST))) (. .))");
    boost::shared_ptr<HyperGraph> act_val(io.ReadFromString(act_str));
    caser_.ToTitle(*act_val);
    return exp_val->CheckEqual(*act_val);
}

int TestCaser::TestHyperGraphTrueCase() {
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
    return 
        exp_val1->CheckEqual(*act_val1) &&
        exp_val2->CheckEqual(*act_val2) &&
        exp_val3->CheckEqual(*act_val3);
}


bool TestCaser::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestWordToLower()" << endl; if(TestWordToLower()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestWordToTitle()" << endl; if(TestWordToTitle()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestWordTrueCase()" << endl; if(TestWordTrueCase()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestSentenceFirst()" << endl; if(TestSentenceFirst()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestSentenceToLower()" << endl; if(TestSentenceToLower()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestSentenceToTitle()" << endl; if(TestSentenceToTitle()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestSentenceTrueCase()" << endl; if(TestSentenceTrueCase()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestHyperGraphToLower()" << endl; if(TestHyperGraphToLower()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestHyperGraphToTitle()" << endl; if(TestHyperGraphToTitle()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestHyperGraphTrueCase()" << endl; if(TestHyperGraphTrueCase()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestCaser Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

