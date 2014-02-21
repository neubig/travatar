#ifndef TEST_HIERO_LOOKUP_TABLE_H__
#define TEST_HIERO_LOOKUP_TABLE_H__

#include "test-base.h"
#include <travatar/lookup-table-hiero.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace boost;

namespace travatar {

class TestLookupTableHiero : public TestBase {
public:

    TestLookupTableHiero() {
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "\"I\" x0 ||| \"watashi\" \"wa\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"eat\" \"two\" x0 ||| \"futatsu\" \"no\" x0 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"two\" x0 ||| \"futatsu\" \"no\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "x0 \"eat\" x1 ||| x0 \"wa\" x1 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"eat\" x0 ||| x0 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" x0 \"two\" \"hamburgers\" ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" x0 \"two\" x1 ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1 \"wo\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" ||| \"watashi\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"eat\" ||| \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"two\" ||| \"futatsu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"hamburgers\" ||| \"hanbaga\" ||| Pegf=0.02 ppen=2.718" << endl;

        istringstream rule_iss_hash(rule_oss.str());

        lookup_hiero.reset(LookupTableHiero::ReadFromRuleTable(rule_iss_hash));
    }

    int TestLookup(LookupTableHiero & lookup) {
        Sentence c = Dict::ParseWords("I eat two");
        vector<TranslationRuleHiero*> result1 = lookup.FindRules(c);
        
        c = Dict::ParseWords("I eat and buy two delicious hamburgers");
        vector<TranslationRuleHiero*> result2 = lookup.FindRules(c);
        
        c = Dict::ParseWords("I eat two hamburgers");
        vector<TranslationRuleHiero*> result3 = lookup.FindRules(c);

        vector<int> act_match_cnt(3,0), exp_match_cnt(3,0);
        act_match_cnt[0] = (int)result1.size();
        act_match_cnt[1] = (int)result2.size();
        act_match_cnt[2] = (int)result3.size();

        exp_match_cnt[0] = 6;
        exp_match_cnt[1] = 9;
        exp_match_cnt[2] = 11;
        
        return CheckVector(exp_match_cnt, act_match_cnt);
    }

    int TestLookupRules(LookupTableHiero & lookup) {
        TranslationRuleHiero* rules[11];
        for (int i=0; i < 11; ++i) {
            rules[i] = new TranslationRuleHiero;
        }

        // Transform into Hiero rule
        vector<string> word, target;
        algorithm::split(word, "\"I\" x0", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" x0", is_any_of(" "));
        rules[0] = (LookupTableHiero::BuildRule(rules[0], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        algorithm::split(word, "\"I\" x0 \"two\" \"hamburgers\"", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0", is_any_of(" "));
        rules[1] = LookupTableHiero::BuildRule(rules[1], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"I\"", is_any_of(" ")); algorithm::split(target, "\"watashi\"", is_any_of(" "));
        rules[2] = LookupTableHiero::BuildRule(rules[2], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"I\" x0 \"two\" x1", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" \"futatsu\" \"no\" x1 \"wo\" x0", is_any_of(" "));
        rules[3] = LookupTableHiero::BuildRule(rules[3], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"eat\" \"two\" x0", is_any_of(" ")); algorithm::split(target, "\"futatsu\" \"no\" x0 \"wo\" \"taberu\"", is_any_of(" "));
        rules[4] = LookupTableHiero::BuildRule(rules[4], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "x0 \"eat\" x1", is_any_of(" ")); algorithm::split(target, "x0 \"wa\" x1 \"wo\" \"taberu\"", is_any_of(" "));
        rules[5] = LookupTableHiero::BuildRule(rules[5], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"eat\" x0", is_any_of(" ")); algorithm::split(target, "x0 \"wo\" \"taberu\"", is_any_of(" "));
        rules[6] = LookupTableHiero::BuildRule(rules[6], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"eat\"", is_any_of(" ")); algorithm::split(target, "\"taberu\"", is_any_of(" "));
        rules[7] = LookupTableHiero::BuildRule(rules[7], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"two\" x0", is_any_of(" ")); algorithm::split(target, "\"futatsu\" \"no\" x0", is_any_of(" "));
        rules[8] = LookupTableHiero::BuildRule(rules[8], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"two\"", is_any_of(" ")); algorithm::split(target, "\"futatsu\"", is_any_of(" "));
        rules[9] = LookupTableHiero::BuildRule(rules[9], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        algorithm::split(word, "\"hamburgers\"", is_any_of(" ")); algorithm::split(target, "\"hanbaga\"", is_any_of(" "));
        rules[10] = LookupTableHiero::BuildRule(rules[10], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        vector<vector<TranslationRuleHiero*> > act_rules(3);
        vector<vector<TranslationRuleHiero*> > exp_rules(3);
        for (int i=0; i < 3 ; ++i) {
            act_rules.push_back(vector<TranslationRuleHiero*>());
            exp_rules.push_back(vector<TranslationRuleHiero*>());
        }

        int exp1[] = {0,2,5,6,7,9};
        int exp2[] = {0,2,3,5,6,7,8,9,10};
        int exp3[] = {0,1,2,3,4,5,6,7,8,9,10};

        act_rules[0] = lookup.FindRules(Dict::ParseWords("I eat two"));
        act_rules[1] = lookup.FindRules(Dict::ParseWords("I eat and buy two delicious hamburgers"));
        act_rules[2] = lookup.FindRules(Dict::ParseWords("I eat two hamburgers"));
        
        BOOST_FOREACH(int rule_number, exp1) {
            exp_rules[0].push_back(rules[rule_number]);
        }
        BOOST_FOREACH(int rule_number, exp2) {
            exp_rules[1].push_back(rules[rule_number]);
        }
        BOOST_FOREACH(int rule_number, exp3) {
            exp_rules[2].push_back(rules[rule_number]);
        }
        int result = CheckSet(act_rules,exp_rules);
        // Janitor time
        for (int i=0; i < 11; ++i) {
            delete rules[i];
        }
        return result;
    }

    bool CheckSet(vector<vector<TranslationRuleHiero*> > & actual, vector<vector<TranslationRuleHiero*> > & expected) {
        bool ret = true;
        for (int i=0; ret && i < (int)actual.size(); ++i) {
            vector<TranslationRuleHiero*> sent_actual = actual[i];
            for (int j=0; ret && j < (int)sent_actual.size(); ++j) {
                TranslationRuleHiero* rule = sent_actual[j];
                int count = 0;
                ret = false;
                vector<TranslationRuleHiero*>::iterator it = expected[i].begin();
                while (it != expected[i].end() && !ret && count++ < (int)expected[i].size()) {
                    if (**it == *rule) {
                        expected[i].erase(it);
                        ret = true;
                    } 
                    ++it;
                }
                if (!ret) {
                    cerr << "NOT FOUND " << rule->ToString() << endl;
                }
            }
        }
        for (int i=0; ret && i < (int) expected.size(); ++i) {
            if (expected[i].size() != 0) {
                ret = false;
            }
        }
        return ret;
    }

    bool TestBuildRules(LookupTableHiero & lookup) {
        return 1;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestLookup(lookup_hiero)" << endl; if(TestLookup(*lookup_hiero)) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestLookupRules(lookup_hiero)" << endl; if(TestLookupRules(*lookup_hiero)) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestBuildRules(lookup_hiero)" << endl; if (TestBuildRules(*lookup_hiero)) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestLookupTableHiero Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }
private:
    boost::scoped_ptr<LookupTableHiero> lookup_hiero;

};
}

#endif
