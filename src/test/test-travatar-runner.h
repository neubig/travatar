#ifndef TEST_TRAVATAR_RUNNER_H__
#define TEST_TRAVATAR_RUNNER_H__

#include "test-base.h"
#include <travatar/travatar-runner.h>

namespace travatar {

class TestTravatarRunner : public TestBase {

public:
    TestTravatarRunner();
    ~TestTravatarRunner();

    bool RunTest();

private:
    //TravatarRunner nr;

};

}

#endif
