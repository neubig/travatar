#ifndef TEST_BINARIZER_H__
#define TEST_BINARIZER_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace travatar {

class TestBinarizer : public TestBase {

public:
    TestBinarizer();
    ~TestBinarizer();

    int TestBinarizerRight();
    int TestBinarizerRightRaisePunc();
    int TestBinarizerUnordered();
    int TestBinarizerLeft();
    int TestBinarizerCKY();
    int TestDoubleRight();
    bool RunTest();

private:
    boost::scoped_ptr<HyperGraph> trinary_graph_, unordered_graph_, double_graph_;
    std::vector<WordId> src_;

};

}

#endif
