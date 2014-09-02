#include "test-caser.h"

#include <travatar/dict.h>

#include <vector>

using namespace std;
using namespace boost;

namespace travatar {

TestCaser::TestCaser() { }
TestCaser::~TestCaser() { }

int TestCaser::TestWordToLower() {
    return 0;
}

int TestCaser::TestWordToTitle() {
    return 0;
}

int TestCaser::TestWordTrueCase() {
    return 0;
}

int TestCaser::TestSentenceFirst() {
    return 0;
}

int TestCaser::TestSentenceToLower() {
    return 0;
}

int TestCaser::TestSentenceToTitle() {
    return 0;
}

int TestCaser::TestSentenceTrueCase() {
    return 0;
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

