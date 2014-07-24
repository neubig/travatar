#ifndef TEST_LM_COMPOSER_H__
#define TEST_LM_COMPOSER_H__

#include "test-base.h"

#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace travatar {

class TestLMComposer : public TestBase {

public:
    TestLMComposer();
    ~TestLMComposer();

    int TestLMComposerBU();
    int TestLMComposerIncremental();
    int TestLMComposerIncrementalTimesTwo();
    int TestReverseBU();
    int TestReverseIncremental();

    bool RunTest();

private:
    // PennTreeIO tree_io_;
    // JSONTreeIO json_tree_io_;
    // std::vector<WordId> src_;
    // boost::scoped_ptr<HyperGraph> rule_graph_, unary_graph_;
    std::string file_name_;
    boost::shared_ptr<HyperGraph> rule_graph_;
    boost::shared_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10, rule_01bad;
    boost::shared_ptr<HyperGraph> exp_graph;
};

}

#endif
