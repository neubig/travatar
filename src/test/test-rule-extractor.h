#ifndef TEST_RULE_EXTRACTOR_H__
#define TEST_RULE_EXTRACTOR_H__

#include "test-base.h"
#include <travatar/forest-extractor.h>
#include <travatar/hyper-graph.h>
#include <travatar/hiero-extractor.h>
#include <travatar/alignment.h>
#include <travatar/rule-composer.h>
#include <travatar/tree-io.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestRuleExtractor : public TestBase {

public:
    TestRuleExtractor();
    ~TestRuleExtractor();

    int TestTreeExtraction();
    int TestForestExtraction();
    int TestForestExtractionBinarized();
    int TestTopNullExtraction();
    int TestExpandNode();
    int TestExhaustiveNullExtraction();
    int TestExhaustiveNullDisconnected();
    int TestRulePrinting();
    int TestRulePrintingTrgSyntax();
    int TestComposeEdge();
    int TestRuleComposer();
    int TestRuleComposerLex();
    int TestTrinary();

    bool RunTest();

private:
    PennTreeIO tree_io;
    JSONTreeIO json_io;
    boost::scoped_ptr<HyperGraph> src1_graph, src2_graph, src3_graph, src4_graph, trg1_graph;
    Sentence trg1_sent, trg2_sent, trg3_sent, trg4_sent;
    Alignment align1, align2, align3, align4;
};

}

#endif
