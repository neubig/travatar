#ifndef TEST_IO_UTIL_H__
#define TEST_IO_UTIL_H__

#include "test-base.h"
#include <travatar/io-util.h>

namespace travatar {

class TestIOUtil : public TestBase {

public:
    TestIOUtil();
    ~TestIOUtil();

    int TestTrim();
    int TestReadUntil();

    bool RunTest();

private:

};

}

#endif
