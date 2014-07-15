#ifndef TEST_HYPER_GRAPH_H__
#define TEST_HYPER_GRAPH_H__

#include <fstream>
#include <utility>
#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/translation-rule.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestHyperGraph : public TestBase {

public:
    TestHyperGraph();
    ~TestHyperGraph();

    int TestCalculateSpan();
    int TestInsideOutside();
    int TestInsideOutsideUnbalanced();
    int TestCopy();
    int TestCalculateSpanForest();
    int TestCalculateFrontier();
    int TestCalculateNull();
    int TestCalculateFrontierForest();
    int TestNbestPath();
    int TestNbestTied();
    int TestPathTranslation();
//    int TestLMIntersection();
    int TestAppend();
    int TestGetLabeledSpans();

    bool RunTest();

private:
    PennTreeIO tree_io;
    boost::scoped_ptr<HyperGraph> src1_graph, src2_graph, src3_graph, rule_graph_;
    Sentence trg1_sent, trg2_sent, trg3_sent;
    Alignment align1, align2, align3;
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;

};

}

#endif
