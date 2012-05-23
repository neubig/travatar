#ifndef TEST_TRABATAR_RUNNER_H__
#define TEST_TRABATAR_RUNNER_H__

#include "test-base.h"
#include <trabatar/trabatar-runner.h>

namespace trabatar {

class TestTrabatarRunner : public TestBase {

public:

    TestTrabatarRunner() { }
    ~TestTrabatarRunner() { }

    bool RunTest() {
        int done = 0, succeeded = 0;
        // done++; cout << "TestLoadCorpus()" << endl; if(TestLoadCorpus()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTrabatarRunner Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    TrabatarRunner nr;

};

}

#endif
