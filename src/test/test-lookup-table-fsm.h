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

    TranslationRuleHiero* BuildRule(const std::string & src, const std::string & trg, const std::string & feat);
    HyperGraph * CreateExpectedGraph(bool extra=false);
    HyperGraph * CreateUnkExpectedGraph(bool del_unk);
    HyperGraph * CreateMultiHeadExpectedGraph();
    bool TestBuildRules(LookupTableFSM & lookup,bool extra = false);
    bool TestUnkRules(LookupTableFSM & lookup, bool del_unk); 
    bool TestMultiHead(LookupTableFSM & lookup);
    bool RunTest();
    
private:
    boost::scoped_ptr<LookupTableFSM> lookup_fsm;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_split;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_extra;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_c;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_mhd;
};
}

#endif
