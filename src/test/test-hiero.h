#ifndef TEST_HIERO_H__  
#define TEST_HIERO_H__

#include "test-base.h"
#include <travatar/rule-extractor.h>
#include <travatar/alignment.h>

using namespace boost;

namespace travatar {

class TestHiero : public TestBase {

public:

    TestHiero() { }

    int TestPhraseExtraction() {
        return 1;
    }

    int TestRuleExtraction() {
        return 1;
    }
        

    ~TestHiero() { }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestPhraseExtraction()" << endl; if(TestPhraseExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRuleExtraction()" << endl; if(TestRuleExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestHiero Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }
};

}

#endif
