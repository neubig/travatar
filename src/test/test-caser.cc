#include "test-caser.h"

#include <travatar/dict.h>
#include <travatar/check-equal.h>

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
    Sentence act_val1 = Dict::ParseWords("this is a test .");
    caser_.TrueCase(act_val1);
    Sentence exp_val2 = Dict::ParseWords("that is a test .");
    Sentence act_val2 = Dict::ParseWords("that is a test .");
    caser_.TrueCase(act_val2);
    Sentence exp_val3 = Dict::ParseWords("Black is a test .");
    Sentence act_val3 = Dict::ParseWords("Black is a test .");
    caser_.TrueCase(act_val3);
    return 
        CheckEqual(exp_val1, act_val1) &&
        CheckEqual(exp_val2, act_val2) &&
        CheckEqual(exp_val3, act_val3);
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
    cout << "#### TestCaser Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

