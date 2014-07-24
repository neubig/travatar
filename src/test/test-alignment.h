#ifndef TEST_ALIGNMENT_H__
#define TEST_ALIGNMENT_H__

#include "test-base.h"
#include <travatar/alignment.h>

namespace travatar {

class TestAlignment : public TestBase {

public:
    TestAlignment();
    ~TestAlignment();

    int TestSrcAlignments();
    int TestReadWrite();

    bool RunTest();

private:
    Alignment align;
};

}

#endif
