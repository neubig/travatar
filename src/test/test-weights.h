#ifndef TEST_WEIGHTS_H__
#define TEST_WEIGHTS_H__

#include "test-base.h"
namespace travatar {

class TestWeights : public TestBase {
public:
    TestWeights();
    ~TestWeights();

    int TestPerceptronUpdate();
    int TestAvgPerceptronUpdate();

    bool RunTest();
};

}

#endif
