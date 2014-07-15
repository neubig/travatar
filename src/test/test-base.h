#ifndef TEST_BASE__
#define TEST_BASE__

namespace travatar {

class TestBase {

public:

    TestBase() : passed_(false) { }
    virtual ~TestBase() { }

    // RunTest must be implemented by any test, and returns true if all
    // tests were passed
    virtual bool RunTest() = 0;

protected:

    bool passed_;

};

}

#endif
