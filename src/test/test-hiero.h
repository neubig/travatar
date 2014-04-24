#ifndef TEST_HIERO_H__  
#define TEST_HIERO_H__

#include "test-base.h"
#include <travatar/rule-extractor.h>
#include <travatar/alignment.h>
#include <travatar/dict.h>
#include <travatar/sentence.h>
#include <travatar/hiero-rule-table.h>
#include <boost/foreach.hpp>

#define HIERO_NUMBER_OF_TEST 2
using namespace boost;


namespace travatar {

class TestHiero : public TestBase {

public:
    TestHiero() {
        extractor = HieroExtractor();
        extractor.SetMaxInitalPhrase(10);
        extractor.SetMaxRuleLen(5);

        english[0] = Dict::ParseWords("I eat rice");
        english[1] = Dict::ParseWords("the hotel front desk");

        japan[0] = Dict::ParseWords("watashi wa gohan o tabemasu");
        japan[1] = Dict::ParseWords("hoteru no uketsuke");

        align[0] = Alignment::FromString("0-0 1-4 2-2");
        align[1] = Alignment::FromString("1-0 2-2 3-2");
    }

    int TestPhraseExtraction() {
        // EXPECTED
        std::vector<set<string> > _exp;

        for (int i=0; i < HIERO_NUMBER_OF_TEST; ++i) {
            _exp.push_back(set<string>());
        }

        _exp[0].insert("I -> watashi");
        _exp[0].insert("I -> watashi wa");
        _exp[0].insert("I eat rice -> watashi wa gohan o tabemasu");
        _exp[0].insert("eat -> tabemasu");
        _exp[0].insert("eat -> o tabemasu");
        _exp[0].insert("eat rice -> gohan o tabemasu");
        _exp[0].insert("eat rice -> wa gohan o tabemasu");
        _exp[0].insert("rice -> gohan");
        _exp[0].insert("rice -> gohan o");
        _exp[0].insert("rice -> wa gohan");
        _exp[0].insert("rice -> wa gohan o");

        _exp[1].insert("the hotel -> hoteru");
        _exp[1].insert("the hotel -> hoteru no");
        _exp[1].insert("the hotel front desk -> hoteru no uketsuke");
        _exp[1].insert("hotel -> hoteru");
        _exp[1].insert("hotel -> hoteru no");
        _exp[1].insert("hotel front desk -> hoteru no uketsuke");
        _exp[1].insert("front desk -> uketsuke");
        _exp[1].insert("front desk -> no uketsuke");

        // TESTING
        for (int i=0; i < HIERO_NUMBER_OF_TEST; ++i) {
            PhrasePairs pps = extractor.ExtractPhrase(align[i],english[i],japan[i]);
            BOOST_FOREACH(PhrasePair pp, pps) {
                string sentence = extractor.PrintPhrasePair(pp,english[i],japan[i]);
                if (_exp[i].find(sentence) == _exp[i].end()) {
                    cerr << "Could not find phrase-pair: \"" + sentence + "\" in expected sentences" << endl;
                    return 0;
                } else {
                    _exp[i].erase(sentence);
                }
            }
            if(_exp[i].size() != (unsigned) 0) {
                cerr << "There are some underived phrases from phrase extraction:" << endl;
                BOOST_FOREACH(string k, _exp[i]) {
                    cerr << "  " << k << endl;
                }
                return 0;
            }
        }
        return 1;
    }

    int TestRuleExtraction() {
         // EXPECTED
        std::vector<set<string> > _exp;

        for (int i=0; i < HIERO_NUMBER_OF_TEST; ++i) {
            _exp.push_back(set<string>());
        }

        _exp[0].insert("\"I\" @ X ||| \"watashi\" @ X");
        _exp[0].insert("\"I\" \"eat\" \"rice\" @ X ||| \"watashi\" \"wa\" \"gohan\" \"o\" \"tabemasu\" @ X");
        _exp[0].insert("x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"o\" \"tabemasu\" @ X");
        _exp[0].insert("x0:X \"eat\" \"rice\" @ X ||| x0:X \"wa\" \"gohan\" \"o\" \"tabemasu\" @ X");
        _exp[0].insert("\"I\" x0:X \"rice\" @ X ||| \"watashi\" \"wa\" \"gohan\" \"o\" x0:X @ X");
        _exp[0].insert("\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X");
        _exp[0].insert("\"I\" \"eat\" x0:X @ X ||| \"watashi\" \"wa\" x0:X \"o\" \"tabemasu\" @ X");
        _exp[0].insert("\"eat\" @ X ||| \"tabemasu\" @ X");
        _exp[0].insert("\"eat\" \"rice\" @ X ||| \"gohan\" \"o\" \"tabemasu\" @ X");
        _exp[0].insert("x0:X \"rice\" @ X ||| \"gohan\" \"o\" x0:X @ X");
        _exp[0].insert("\"eat\" x0:X @ X ||| x0:X \"o\" \"tabemasu\" @ X");
        _exp[0].insert("\"rice\" @ X ||| \"gohan\" @ X");
        
        _exp[1].insert("\"the\" \"hotel\" @ X ||| \"hoteru\" @ X");
        _exp[1].insert("\"the\" \"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X");
        _exp[1].insert("x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X");
        _exp[1].insert("\"the\" x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X");
        _exp[1].insert("\"the\" \"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X");
        _exp[1].insert("\"hotel\" @ X ||| \"hoteru\" @ X");
        _exp[1].insert("\"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X");
        _exp[1].insert("\"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X");
        _exp[1].insert("\"front\" \"desk\" @ X ||| \"uketsuke\" @ X");

        // TESTING
        for (int i=0; i < HIERO_NUMBER_OF_TEST; ++i) {
            std::vector<vector<HieroRule> >rules = extractor.ExtractHieroRule(align[i],english[i],japan[i]);
            std::set<string> checked = std::set<string>();

            BOOST_FOREACH(vector<HieroRule> rule , rules) {
                BOOST_FOREACH(HieroRule r , rule) {
                    string rule_key = r.ToString();
                    set<string>::iterator it = _exp[i].find(rule_key);

                    if (it == _exp[i].end() && checked.find(rule_key) == checked.end()) {
                        cerr << "Could not find rule: " + rule_key + " in expected rules" << endl;
                        return 0;
                    } else {
                        _exp[i].erase(rule_key);
                        checked.insert(rule_key);
                    }

                }
            }
            if (_exp[i].size() > (unsigned)0) {
                cerr << "There are some underived rules from rule extraction:" << endl;
                BOOST_FOREACH(string k, _exp[i]) {
                    cerr << "  " << k << endl;
                }
                return 0;
            }
        }
        return 1;
    }
        

    ~TestHiero() { }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestPhraseExtraction()" << endl; if(TestPhraseExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRuleExtraction()" << endl; if(TestRuleExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestHiero Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    HieroExtractor extractor;
    Sentence english[HIERO_NUMBER_OF_TEST];
    Sentence japan[HIERO_NUMBER_OF_TEST];
    Alignment align[HIERO_NUMBER_OF_TEST];
};
}

#endif
