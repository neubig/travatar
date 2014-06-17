#ifndef TEST_HIERO_H__  
#define TEST_HIERO_H__

#include "test-base.h"
#include <travatar/rule-extractor.h>
#include <travatar/alignment.h>
#include <travatar/dict.h>
#include <travatar/sentence.h>
#include <travatar/hiero-rule-table.h>
#include <boost/foreach.hpp>

using namespace boost;


namespace travatar {

class TestHiero : public TestBase {

public:
    TestHiero() {
        extractor = HieroExtractor();
        extractor.SetMaxInitialPhrase(10);
        extractor.SetMaxRuleLen(5);

        english.push_back(Dict::ParseWords("I eat rice"));
        japan.push_back(Dict::ParseWords("watashi wa gohan o tabemasu"));
        align.push_back(Alignment::FromString("0-0 1-4 2-2"));

        english.push_back(Dict::ParseWords("the hotel front desk"));
        japan.push_back(Dict::ParseWords("hoteru no uketsuke"));
        align.push_back(Alignment::FromString("1-0 2-2 3-2"));
    }

    // Test phrase extraction using the current settings of the phrase extraction
    int PhraseTest(std::vector<set<string> > & exp) {
        for (int i=0; i < (int)exp.size(); ++i) {
            PhrasePairs pps = extractor.ExtractPhrase(align[i],english[i],japan[i]);
            BOOST_FOREACH(PhrasePair pp, pps) {
                string sentence = extractor.PrintPhrasePair(pp,english[i],japan[i]);
                if (exp[i].find(sentence) == exp[i].end()) {
                    cerr << "Could not find phrase-pair: \"" + sentence + "\" in expected sentences" << endl;
                    return 0;
                } else {
                    exp[i].erase(sentence);
                }
            }
            if(exp[i].size() != (unsigned) 0) {
                cerr << "There are some underived phrases from phrase extraction:" << endl;
                BOOST_FOREACH(string k, exp[i]) {
                    cerr << "  " << k << endl;
                }
                return 0;
            }
        }
        return 1;
    }

    int TestPhraseExtraction() {
        // EXPECTED
        std::vector<set<string> > exp;

        exp.push_back(set<string>());
        exp[0].insert("I -> watashi");
        exp[0].insert("I -> watashi wa");
        exp[0].insert("I eat rice -> watashi wa gohan o tabemasu");
        exp[0].insert("eat -> tabemasu");
        exp[0].insert("eat -> o tabemasu");
        exp[0].insert("eat rice -> gohan o tabemasu");
        exp[0].insert("eat rice -> wa gohan o tabemasu");
        exp[0].insert("rice -> gohan");
        exp[0].insert("rice -> gohan o");
        exp[0].insert("rice -> wa gohan");
        exp[0].insert("rice -> wa gohan o");

        exp.push_back(set<string>());
        exp[1].insert("the hotel -> hoteru");
        exp[1].insert("the hotel -> hoteru no");
        exp[1].insert("the hotel front desk -> hoteru no uketsuke");
        exp[1].insert("hotel -> hoteru");
        exp[1].insert("hotel -> hoteru no");
        exp[1].insert("hotel front desk -> hoteru no uketsuke");
        exp[1].insert("front desk -> uketsuke");
        exp[1].insert("front desk -> no uketsuke");

        return PhraseTest(exp);
    }

    int TestPhraseExtractionLimit() {
        // EXPECTED
        std::vector<set<string> > exp;

        extractor.SetMaxInitialPhrase(3);

        exp.push_back(set<string>());
        exp[0].insert("I -> watashi");
        exp[0].insert("I -> watashi wa");
        exp[0].insert("eat -> tabemasu");
        exp[0].insert("eat -> o tabemasu");
        exp[0].insert("eat rice -> gohan o tabemasu");
        exp[0].insert("rice -> gohan");
        exp[0].insert("rice -> gohan o");
        exp[0].insert("rice -> wa gohan");
        exp[0].insert("rice -> wa gohan o");

        exp.push_back(set<string>());
        exp[1].insert("the hotel -> hoteru");
        exp[1].insert("the hotel -> hoteru no");
        exp[1].insert("hotel -> hoteru");
        exp[1].insert("hotel -> hoteru no");
        exp[1].insert("hotel front desk -> hoteru no uketsuke");
        exp[1].insert("front desk -> uketsuke");
        exp[1].insert("front desk -> no uketsuke");

        int ret = PhraseTest(exp);
        extractor.SetMaxInitialPhrase(10);

        return ret;
    }

    int TestRuleExtraction() {
         // EXPECTED
        std::vector<set<string> > exp;

        exp.push_back(set<string>());
        exp[0].insert("\"I\" @ X ||| \"watashi\" @ X");
        exp[0].insert("\"I\" \"eat\" \"rice\" @ X ||| \"watashi\" \"wa\" \"gohan\" \"o\" \"tabemasu\" @ X");
        exp[0].insert("x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"o\" \"tabemasu\" @ X");
        exp[0].insert("x0:X \"eat\" \"rice\" @ X ||| x0:X \"wa\" \"gohan\" \"o\" \"tabemasu\" @ X");
        exp[0].insert("\"I\" x0:X \"rice\" @ X ||| \"watashi\" \"wa\" \"gohan\" \"o\" x0:X @ X");
        exp[0].insert("\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X");
        exp[0].insert("\"I\" \"eat\" x0:X @ X ||| \"watashi\" \"wa\" x0:X \"o\" \"tabemasu\" @ X");
        exp[0].insert("\"eat\" @ X ||| \"tabemasu\" @ X");
        exp[0].insert("\"eat\" \"rice\" @ X ||| \"gohan\" \"o\" \"tabemasu\" @ X");
        exp[0].insert("x0:X \"rice\" @ X ||| \"gohan\" \"o\" x0:X @ X");
        exp[0].insert("\"eat\" x0:X @ X ||| x0:X \"o\" \"tabemasu\" @ X");
        exp[0].insert("\"rice\" @ X ||| \"gohan\" @ X");
        
        exp.push_back(set<string>());
        exp[1].insert("\"the\" \"hotel\" @ X ||| \"hoteru\" @ X");
        exp[1].insert("\"the\" \"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X");
        exp[1].insert("x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X");
        exp[1].insert("\"the\" x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X");
        exp[1].insert("\"the\" \"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X");
        exp[1].insert("\"hotel\" @ X ||| \"hoteru\" @ X");
        exp[1].insert("\"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X");
        exp[1].insert("\"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X");
        exp[1].insert("\"front\" \"desk\" @ X ||| \"uketsuke\" @ X");

        // TESTING
        for (int i=0; i < (int)exp.size(); ++i) {
            std::vector<vector<HieroRule> >rules = extractor.ExtractHieroRule(align[i],english[i],japan[i]);
            std::set<string> checked = std::set<string>();

            BOOST_FOREACH(vector<HieroRule> rule , rules) {
                BOOST_FOREACH(HieroRule r , rule) {
                    string rule_key = r.ToString();
                    set<string>::iterator it = exp[i].find(rule_key);

                    if (it == exp[i].end() && checked.find(rule_key) == checked.end()) {
                        cerr << "Could not find rule: " + rule_key + " in expected rules" << endl;
                        return 0;
                    } else {
                        exp[i].erase(rule_key);
                        checked.insert(rule_key);
                    }

                }
            }
            if (exp[i].size() > (unsigned)0) {
                cerr << "There are some underived rules from rule extraction:" << endl;
                BOOST_FOREACH(string k, exp[i]) {
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
        done++; cout << "TestPhraseExtractionLimit()" << endl; if(TestPhraseExtractionLimit()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRuleExtraction()" << endl; if(TestRuleExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestHiero Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    HieroExtractor extractor;
    std::vector<Sentence> english;
    std::vector<Sentence> japan;
    std::vector<Alignment> align;
};
}

#endif
