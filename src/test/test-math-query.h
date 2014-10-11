#ifndef TEST_MATH_QUERY_H__
#define TEST_MATH_QUERY_H__

#include "test-base.h"
#include <travatar/math-query.h>

namespace travatar {

class TestMathQuery : public TestBase {
public:
    TestMathQuery();
    ~TestMathQuery() { }
    
    bool TestReadQueryAdd();
    bool TestEvalQueryBasic();
    bool TestEvalQueryIntermediate();
    bool TestEvalQueryAdvance();
    bool TestEvalQueryFailOpen();
    bool TestEvalQueryFailClosed();
    bool TestEvalQueryFailOperator();
    bool TestEvalQueryFailOperand();
    bool TestEvalQueryFailVariable();
    bool TestInvalidQuery();
    bool RunTest();
    
private:
};
}

#endif
