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
        rule_oss << "\"eat\" x0 ||| x0 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" x0 \"two\" \"hamburgers\" ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0 ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"I\" ||| \"watashi\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"eat\" ||| \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"two\" ||| \"futatsu\" ||| Pegf=0.02 ppen=2.718" << endl;
        rule_oss << "\"hamburgers\" ||| \"hanbaga\" ||| Pegf=0.02 ppen=2.718" << endl;

        istringstream rule_iss_hash(rule_oss.str());

        lookup_hiero.reset(LookupTableHiero::ReadFromRuleTable(rule_iss_hash));
        cerr << lookup_hiero->ToString() << endl;
    }

    int TestLookup(LookupTableHiero & lookup) {
        vector<int> act_match_cnt(4, 0), exp_match_cnt(4, 0);
        for (int i=0; i < (int)src1_sent.size(); ++i) {
            vector<TranslationRuleHiero*> expected = lookup.FindRules(src1_sent[i]);
            act_match_cnt[i] = (expected.size());
        }
        exp_match_cnt[0] = 3; // I 
        exp_match_cnt[1] = 4; // eat
        exp_match_cnt[2] = 2; // two
        exp_match_cnt[3] = 1; // hamburger
        return CheckVector(exp_match_cnt, act_match_cnt);
    }

    int TestLookupRules(LookupTableHiero & lookup) {
        vector<vector<TranslationRuleHiero*> > act_rules(4);
        vector<vector<TranslationRuleHiero*> > exp_rules(4);
        for (int i=0; i < 4 ; ++i) {
            act_rules.push_back(vector<TranslationRuleHiero*>());
            exp_rules.push_back(vector<TranslationRuleHiero*>());
        }

        for (int i=0; i < (int)src1_sent.size(); ++i) {
            vector<TranslationRuleHiero*> founded = lookup.FindRules(src1_sent[i]);
            BOOST_FOREACH(TranslationRuleHiero* rule, founded) {
                act_rules[i].push_back(rule);
            }
        }
        // expected rules
        TranslationRuleHiero* hrule = new TranslationRuleHiero();
        vector<string> word, target;
        algorithm::split(word, "\"I\" x0", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" x0", is_any_of(" "));
        exp_rules[0].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"I\" x0 \"two\" \"hamburgers\"", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0", is_any_of(" "));
        exp_rules[0].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"I\"", is_any_of(" ")); algorithm::split(target, "\"watashi\"", is_any_of(" "));
        exp_rules[0].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"eat\" \"two\" x0", is_any_of(" ")); algorithm::split(target, "\"futatsu\" \"no\" x0 \"wo\" \"taberu\"", is_any_of(" "));
        exp_rules[1].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
       
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "x0 \"eat\" x1", is_any_of(" ")); algorithm::split(target, "x0 \"wa\" x1 \"wo\" \"taberu\"", is_any_of(" "));
        exp_rules[1].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"eat\" x0", is_any_of(" ")); algorithm::split(target, "x0 \"wo\" \"taberu\"", is_any_of(" "));
        exp_rules[1].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));

        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"eat\"", is_any_of(" ")); algorithm::split(target, "\"taberu\"", is_any_of(" "));
        exp_rules[1].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"two\" x0", is_any_of(" ")); algorithm::split(target, "\"futatsu\" \"no\" x0", is_any_of(" "));
        exp_rules[2].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"two\"", is_any_of(" ")); algorithm::split(target, "\"futatsu\"", is_any_of(" "));
        exp_rules[2].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        hrule = new TranslationRuleHiero();
        algorithm::split(word, "\"hamburgers\"", is_any_of(" ")); algorithm::split(target, "\"hanbaga\"", is_any_of(" "));
        exp_rules[3].push_back(LookupTableHiero::BuildRule(hrule, word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        int result = CheckSetAndCleanUp(act_rules,exp_rules);
        // Clean up!
        BOOST_FOREACH(vector<TranslationRuleHiero*> vt, exp_rules) {
            BOOST_FOREACH(TranslationRuleHiero* t, vt) {
                delete t;
            }
        }
        return result;
    }

    bool CheckSetAndCleanUp(vector<vector<TranslationRuleHiero*> > & actual, vector<vector<TranslationRuleHiero*> > & expected) {
        bool ret = true;
        for (int i=0; ret && i < (int)actual.size(); ++i) {
            vector<TranslationRuleHiero*> word_actual = actual[i];
            for (int j=0; ret && j < (int)word_actual.size(); ++j) {
                TranslationRuleHiero* rule = word_actual[j];
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
        scoped_ptr<HyperGraph> hg(lookup.BuildHyperGraph("I eat two hamburgers"));
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
    Sentence src1_sent;
    Sentence trg1_sent;
    Alignment align1;

};
}

#endif
