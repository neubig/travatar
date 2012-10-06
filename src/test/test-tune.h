#ifndef TEST_TUNER_H__
#define TEST_TUNER_H__

#include "test-base.h"
#include <travatar/tuner.h>
#include <travatar/dict.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestTune : public TestBase {

public:

    TestTune() { }
    ~TestTune() { }

    int TestAveragePerceptronUpdate() {
        // Update weights
        NbestList nbest2;
        return 0;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestAveragePerceptronUpdate()" << endl; if(TestAveragePerceptronUpdate()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTune Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
