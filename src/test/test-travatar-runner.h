#ifndef TEST_TRAVATAR_RUNNER_H__
#define TEST_TRAVATAR_RUNNER_H__

#include "test-base.h"
#include <travatar/travatar-runner.h>

namespace travatar {

class TestTravatarRunner : public TestBase {

public:

    TestTravatarRunner() { }
    ~TestTravatarRunner() { }

    bool RunTest() {
        int done = 0, succeeded = 0;
        // done++; cout << "TestLoadCorpus()" << endl; if(TestLoadCorpus()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTravatarRunner Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    TravatarRunner nr;

};

}

#endif
