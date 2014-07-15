#ifndef TEST_DICT_H__
#define TEST_DICT_H__

#include "test-base.h"
#include <travatar/dict.h>

namespace travatar {

class TestDict : public TestBase {

public:
    TestDict();
    ~TestDict();

    int TestParseWords();
    int TestParseFeatures();

    bool RunTest();
};

}

#endif
