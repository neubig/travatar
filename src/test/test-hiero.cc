#include "test-hiero.h"

namespace travatar {

TestHiero::TestHiero() {
    extractor = HieroExtractor();
    extractor.SetMaxInitialPhrase(10);
    extractor.SetMaxTerminals(5);

    english.push_back(Dict::ParseWords("I eat rice"));
    japan.push_back(Dict::ParseWords("watashi wa gohan o tabemasu"));
    align.push_back(Alignment::FromString("0-0 1-4 2-2"));

    english.push_back(Dict::ParseWords("the hotel front desk"));
    japan.push_back(Dict::ParseWords("hoteru no uketsuke"));
    align.push_back(Alignment::FromString("1-0 2-2 3-2"));
}

TestHiero::~TestHiero() { }

// Test phrase extraction using the current settings of the phrase extraction
int TestHiero::PhraseTest(std::vector<set<string> > & exp) {
    for (int i=0; i < (int)exp.size(); ++i) {
        HieroExtractor::PhrasePairs pps = extractor.ExtractPhrase(align[i],english[i],japan[i]);
        BOOST_FOREACH(HieroExtractor::PhrasePair pp, pps) {
            string sentence = HieroExtractor::PrintPhrasePair(pp,english[i],japan[i]);
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

int TestHiero::TestPhraseExtraction() {
    // EXPECTED
    std::vector< set<string> > exp;

    exp.push_back(set<string>());
    exp[0].insert("I ||| watashi");
    exp[0].insert("I ||| watashi wa");
    exp[0].insert("I eat rice ||| watashi wa gohan o tabemasu");
    exp[0].insert("eat ||| tabemasu");
    exp[0].insert("eat ||| o tabemasu");
    exp[0].insert("eat rice ||| gohan o tabemasu");
    exp[0].insert("eat rice ||| wa gohan o tabemasu");
    exp[0].insert("rice ||| gohan");
    exp[0].insert("rice ||| gohan o");
    exp[0].insert("rice ||| wa gohan");
    exp[0].insert("rice ||| wa gohan o");

    exp.push_back(set<string>());
    exp[1].insert("the hotel ||| hoteru");
    exp[1].insert("the hotel ||| hoteru no");
    exp[1].insert("the hotel front desk ||| hoteru no uketsuke");
    exp[1].insert("hotel ||| hoteru");
    exp[1].insert("hotel ||| hoteru no");
    exp[1].insert("hotel front desk ||| hoteru no uketsuke");
    exp[1].insert("front desk ||| uketsuke");
    exp[1].insert("front desk ||| no uketsuke");

    return PhraseTest(exp);
}

int TestHiero::TestPhraseExtractionLimit() {
    // EXPECTED
    std::vector< set<string> > exp;

    extractor.SetMaxInitialPhrase(3);

    exp.push_back(set<string>());
    exp[0].insert("I ||| watashi");
    exp[0].insert("I ||| watashi wa");
    exp[0].insert("eat ||| tabemasu");
    exp[0].insert("eat ||| o tabemasu");
    exp[0].insert("eat rice ||| gohan o tabemasu");
    exp[0].insert("rice ||| gohan");
    exp[0].insert("rice ||| gohan o");
    exp[0].insert("rice ||| wa gohan");
    exp[0].insert("rice ||| wa gohan o");

    exp.push_back(set<string>());
    exp[1].insert("the hotel ||| hoteru");
    exp[1].insert("the hotel ||| hoteru no");
    exp[1].insert("hotel ||| hoteru");
    exp[1].insert("hotel ||| hoteru no");
    exp[1].insert("hotel front desk ||| hoteru no uketsuke");
    exp[1].insert("front desk ||| uketsuke");
    exp[1].insert("front desk ||| no uketsuke");
     
    int ret = PhraseTest(exp);
    extractor.SetMaxInitialPhrase(10);
    return ret;
}

int TestHiero::RuleTest(std::vector<set<string> > & exp) {
    // TESTING
    for (int i=0; i < (int)exp.size(); ++i) {
        std::vector< vector<HieroRule*> > rules = extractor.ExtractHieroRule(align[i],english[i],japan[i]);
        
        std::set<string> checked = std::set<string>();

        BOOST_FOREACH(vector<HieroRule*> rule , rules) {
            BOOST_FOREACH(HieroRule* r , rule) {
                string rule_key = r->ToString();
                set<string>::iterator it = exp[i].find(rule_key);

                if (it == exp[i].end() && checked.find(rule_key) == checked.end()) {
                    cerr << "Could not find rule: " + rule_key + " in expected rules" << endl;
                    return 0;
                } else {
                    exp[i].erase(rule_key);
                    checked.insert(rule_key);
                }
                delete r;
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

int TestHiero::TestRuleExtraction() {
     // EXPECTED
    std::vector<set<string> > exp;

    exp.push_back(set<string>());
    exp[0].insert("\"I\" @ X ||| \"watashi\" @ X [0-0]");
    exp[0].insert("\"I\" \"eat\" \"rice\" @ X ||| \"watashi\" \"wa\" \"gohan\" \"o\" \"tabemasu\" @ X [0-0 1-4 2-2]");
    exp[0].insert("x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"o\" \"tabemasu\" @ X [0-2]");
    exp[0].insert("x0:X \"eat\" \"rice\" @ X ||| x0:X \"wa\" \"gohan\" \"o\" \"tabemasu\" @ X [0-3 1-1]");
    exp[0].insert("\"I\" x0:X \"rice\" @ X ||| \"watashi\" \"wa\" \"gohan\" \"o\" x0:X @ X [0-0 1-2]");
    exp[0].insert("\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X [0-0]");
    exp[0].insert("\"I\" \"eat\" x0:X @ X ||| \"watashi\" \"wa\" x0:X \"o\" \"tabemasu\" @ X [0-0 1-3]");
    exp[0].insert("\"eat\" @ X ||| \"tabemasu\" @ X [0-0]");
    exp[0].insert("\"eat\" \"rice\" @ X ||| \"gohan\" \"o\" \"tabemasu\" @ X [0-2 1-0]");
    exp[0].insert("x0:X \"rice\" @ X ||| \"gohan\" \"o\" x0:X @ X [0-0]");
    exp[0].insert("\"eat\" x0:X @ X ||| x0:X \"o\" \"tabemasu\" @ X [0-1]");
    exp[0].insert("\"rice\" @ X ||| \"gohan\" @ X [0-0]");
    
    exp.push_back(set<string>());
    exp[1].insert("\"the\" \"hotel\" @ X ||| \"hoteru\" @ X [1-0]");
    exp[1].insert("\"the\" \"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X [1-0 2-2 3-2]");
    exp[1].insert("x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X [0-1 1-1]");
    exp[1].insert("\"the\" x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X [1-1 2-1]");
    exp[1].insert("\"the\" \"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X [1-0]");
    exp[1].insert("\"hotel\" @ X ||| \"hoteru\" @ X [0-0]");
    exp[1].insert("\"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X [0-0 1-2 2-2]");
    exp[1].insert("\"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X [0-0]");
    exp[1].insert("\"front\" \"desk\" @ X ||| \"uketsuke\" @ X [0-0 1-0]");

    return RuleTest(exp);
}

int TestHiero::TestRuleExtractionInitial() {
     // EXPECTED
    std::vector<set<string> > exp;

    extractor.SetMaxInitialPhrase(3);
    exp.push_back(set<string>());
    exp[0].insert("\"I\" @ X ||| \"watashi\" @ X [0-0]");
    exp[0].insert("\"eat\" @ X ||| \"tabemasu\" @ X [0-0]");
    exp[0].insert("\"eat\" \"rice\" @ X ||| \"gohan\" \"o\" \"tabemasu\" @ X [0-2 1-0]");
    exp[0].insert("x0:X \"rice\" @ X ||| \"gohan\" \"o\" x0:X @ X [0-0]");
    exp[0].insert("\"eat\" x0:X @ X ||| x0:X \"o\" \"tabemasu\" @ X [0-1]");
    exp[0].insert("\"rice\" @ X ||| \"gohan\" @ X [0-0]");
    
    exp.push_back(set<string>());
    exp[1].insert("\"the\" \"hotel\" @ X ||| \"hoteru\" @ X [1-0]");
    exp[1].insert("x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X [0-1 1-1]");
    exp[1].insert("\"hotel\" @ X ||| \"hoteru\" @ X [0-0]");
    exp[1].insert("\"hotel\" \"front\" \"desk\" @ X ||| \"hoteru\" \"no\" \"uketsuke\" @ X [0-0 1-2 2-2]");
    exp[1].insert("\"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X [0-0]");
    exp[1].insert("\"front\" \"desk\" @ X ||| \"uketsuke\" @ X [0-0 1-0]");

    int ret = RuleTest(exp);
    extractor.SetMaxInitialPhrase(10);
    
    return ret;
}

int TestHiero::TestRuleExtractionLen() {
     // EXPECTED
    std::vector<set<string> > exp;

    extractor.SetMaxTerminals(2);
    exp.push_back(set<string>());
    exp[0].insert("\"I\" @ X ||| \"watashi\" @ X [0-0]");
    exp[0].insert("\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X [0-0]");
    exp[0].insert("\"eat\" @ X ||| \"tabemasu\" @ X [0-0]");
    exp[0].insert("x0:X \"rice\" @ X ||| \"gohan\" \"o\" x0:X @ X [0-0]");
    exp[0].insert("\"eat\" x0:X @ X ||| x0:X \"o\" \"tabemasu\" @ X [0-1]");
    exp[0].insert("\"rice\" @ X ||| \"gohan\" @ X [0-0]");

    exp.push_back(set<string>());
    exp[1].insert("\"the\" \"hotel\" @ X ||| \"hoteru\" @ X [1-0]");
    exp[1].insert("x0:X \"front\" \"desk\" @ X ||| x0:X \"no\" \"uketsuke\" @ X [0-1 1-1]");
    exp[1].insert("\"the\" \"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X [1-0]");
    exp[1].insert("\"hotel\" @ X ||| \"hoteru\" @ X [0-0]");
    exp[1].insert("\"hotel\" x0:X @ X ||| \"hoteru\" \"no\" x0:X @ X [0-0]");
    exp[1].insert("\"front\" \"desk\" @ X ||| \"uketsuke\" @ X [0-0 1-0]");

    int ret = RuleTest(exp);
    extractor.SetMaxTerminals(5);
    return ret;
}
    
bool TestHiero::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestPhraseExtraction()" << endl; if(TestPhraseExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestPhraseExtractionLimit()" << endl; if(TestPhraseExtractionLimit()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRuleExtraction()" << endl; if(TestRuleExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRuleExtractionInitial()" << endl; if(TestRuleExtractionInitial()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRuleExtractionLen()" << endl; if(TestRuleExtractionLen()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestHiero Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

