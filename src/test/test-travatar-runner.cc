#include "test-travatar-runner.h"

namespace travatar {

TestTravatarRunner::TestTravatarRunner() { }

TestTravatarRunner::~TestTravatarRunner() { }

bool TestTravatarRunner::RunTest() {
    int done = 0, succeeded = 0;
    // done++; cout << "TestLoadCorpus()" << endl; if(TestLoadCorpus()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestTravatarRunner Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

