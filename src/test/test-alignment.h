#ifndef TEST_ALIGNMENT_H__
#define TEST_ALIGNMENT_H__

#include "test-base.h"
#include <travatar/alignment.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <boost/scoped_ptr.hpp>

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
