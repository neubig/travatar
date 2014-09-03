#ifndef TEST_CASER_H__
#define TEST_CASER_H__

#include "test-base.h"
#include <travatar/caser.h>

namespace travatar {

class TestCaser : public TestBase {

public:
    TestCaser();
    ~TestCaser();


    int TestWordToLower();
    int TestWordToTitle();
    int TestWordTrueCase();
    int TestSentenceFirst();
    int TestSentenceToLower();
    int TestSentenceToTitle(); 
    int TestSentenceTrueCase();
    int TestHyperGraphToLower();
    int TestHyperGraphToTitle(); 
    int TestHyperGraphTrueCase();


    bool RunTest();

protected:

    Caser caser_;

};

}

#endif
