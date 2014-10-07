#ifndef TEST_TOKENIZER_H__
#define TEST_TOKENIZER_H__

#include "test-base.h"
#include <travatar/tokenizer-penn.h>
#include <travatar/tokenizer.h>

namespace travatar {

class TestTokenizer : public TestBase {

public:
    TestTokenizer();
    ~TestTokenizer();

    int TestPenn();

    bool RunTest();

protected:

    TokenizerPenn tokenizer_penn_;

};

}

#endif
