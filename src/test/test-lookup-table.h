#ifndef TEST_LOOKUP_TABLE_H__
#define TEST_LOOKUP_TABLE_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/lookup-table-marisa.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestLookupTable : public TestBase {

public:
    TestLookupTable();
    ~TestLookupTable();

    int TestLookup(LookupTable & lookup);
    int TestLookupRules(LookupTable & lookup);
    int TestBuildRuleGraph();
    int TestBadInputHash();
    int TestBadInputMarisa();
    
    bool RunTest();

private:
    JSONTreeIO tree_io;
    boost::scoped_ptr<LookupTableHash> lookup_hash;
    boost::scoped_ptr<LookupTableMarisa> lookup_marisa;
    boost::scoped_ptr<HyperGraph> src1_graph;
    Sentence trg1_sent;
    Alignment align1;
};

}

#endif
