#ifndef TEST_GRAPH_TRANSFORMER_H__
#define TEST_GRAPH_TRANSFORMER_H__

#include "test-base.h"
#include <travatar/tree-io.h>
#include <utility>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TranslationRule;

class TestGraphTransformer : public TestBase {

public:
    TestGraphTransformer();
    ~TestGraphTransformer();

    int TestUnaryFlatten();
    int TestUnaryFlatten2();
    int TestWordSplit();
    int TestWordSplitConnected();
    int TestWordSplitInitFinal();
    int TestWordSplitSingle();
    int TestCompoundWordSplit();
    int TestCompoundWordSplitFiller();

    bool RunTest();

private:
    PennTreeIO tree_io_;
    JSONTreeIO json_tree_io_;
    std::vector<WordId> src_;
    boost::scoped_ptr<HyperGraph> rule_graph_, unary_graph_;
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

}

#endif
