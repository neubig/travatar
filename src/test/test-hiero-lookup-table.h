#ifndef TEST_HIERO_LOOKUP_TABLE_H__
#define TEST_HIERO_LOOKUP_TABLE_H__

#include "test-base.h"

using namespace boost;

namespace travatar {

class TestLookupTableHiero : public TestBase {
public:

    TestLookupTableHiero() {
        string src1_str = "I eat two hamburgers";
        string trg1_str  = "watashi wa futatsu no hanbaga wo taberu";
        string align1_str = "0-0 1-6 2-2 3-4";
        align1 = Alignment::FromString(align1_str);
        src1_sent = Dict::ParseWords(src1_str);
        trg1_sent = Dict::ParseWords(trg1_str);
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "\"I\" x0 ||| \"watashi\" \"wa\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"eat\" \"two\" x0 ||| \"futatsu\" \"no\" x0 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"two\" x0 ||| \"futatsu\" \"no\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "x0 \"eat\" x1 ||| x0 \"wa\" x1 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" x0 \"two\" \"hamburgers\" ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" ||| \"watashi\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"eat\" ||| \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"two\" ||| \"futatsu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"hamburgers\" ||| \"hamburger\" ||| Pegf=0.02 ppen=2.718" << endl;

        istringstream rule_iss_hash(rule_oss.str());
        lookup_hash.reset(LookupTableHash::ReadFromRuleTable(rule_iss_hash));
    }

    int TestLookup(LookupTable & lookup) {
        vector<int> exp_match_cnt(4, 0), act_match_cnt(4, 0);
        return 1;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        cout << "#### TestLookupTableHiero Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return 1;
    }
private:
    boost::scoped_ptr<LookupTableHash> lookup_hash;
    boost::scoped_ptr<LookupTableMarisa> lookup_marisa;
    boost::scoped_ptr<HyperGraph> src1_graph;
    Sentence src1_sent;
    Sentence trg1_sent;
    Alignment align1;

};
}

#endif
