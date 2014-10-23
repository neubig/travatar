#ifndef TEST_TREE_IO_H__
#define TEST_TREE_IO_H__

#include "test-base.h"

#include <travatar/hyper-graph.h>
#include <string>

namespace travatar {

class TestTreeIO : public TestBase {
public:
    TestTreeIO();
    ~TestTreeIO();

    int TestReadPenn();
    int TestReadPennEmpty();
    int TestReadRule();
    int TestReadEgret();
    int TestWriteEgret();
    int TestReadJSON();
    int TestWriteJSON();
    int TestRoundtripJSON();
    int TestRoundtripJSONQuote();
    int TestWritePenn();
    int TestWriteRule();
    int TestWriteMosesXML();
    int TestReadWord();
    
    bool RunTest();

private:
    std::string tree_str, rule_str, graph_str, egret_str;
    HyperGraph tree_exp, graph_exp, quote_exp, egret_exp;
};

}

#endif
