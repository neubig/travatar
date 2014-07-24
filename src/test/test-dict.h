#ifndef TEST_DICT_H__
#define TEST_DICT_H__

#include "test-base.h"

namespace travatar {

class TestDict : public TestBase {

public:
    TestDict();
    ~TestDict();

    int TestParseWords();
    int TestParseSparseMap();

    bool RunTest();
};

}

#endif
