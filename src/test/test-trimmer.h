#ifndef TEST_TRIMMER_H__
#define TEST_TRIMMER_H__

#include "test-base.h"
#include <travatar/trimmer-nbest.h>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestTrimmer : public TestBase {

public:
    TestTrimmer();
    ~TestTrimmer();

    int TestNbest();
    int TestNbestNode();

    bool RunTest();

protected:

    boost::shared_ptr<HyperGraph> rule_graph_, binary_graph_; 
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

}

#endif
