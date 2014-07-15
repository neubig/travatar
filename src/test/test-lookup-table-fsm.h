#ifndef TEST_LOOKUP_TABLE_FSM_H__
#define TEST_LOOKUP_TABLE_FSM_H__

#include "test-base.h"
#include <travatar/lookup-table-fsm.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

namespace travatar {

class TestLookupTableFSM : public TestBase {
public:
    TestLookupTableFSM();
    ~TestLookupTableFSM();

    boost::shared_ptr<TranslationRuleHiero> BuildRule(const std::string & src, const std::string & trg, const std::string & feat);
    HyperGraph * CreateExpectedGraph();
    bool TestBuildRules(LookupTableFSM & lookup);

    bool RunTest();

private:
    boost::scoped_ptr<LookupTableFSM> lookup_fsm;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_split;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_extra;
};

}

#endif
