#ifndef TEST_HIERO_H__  
#define TEST_HIERO_H__

#include "test-base.h"
#include <travatar/hiero-extractor.h>
#include <travatar/alignment.h>
#include <travatar/dict.h>
#include <travatar/sentence.h>
#include <boost/foreach.hpp>

namespace travatar {

class TestHiero : public TestBase {

public:
    TestHiero();
    ~TestHiero();
    
    int PhraseTest(std::vector<set<string> > & exp);
    int TestPhraseExtraction();
    int TestPhraseExtractionLimit();
    int RuleTest(std::vector<set<string> > & exp);
    int TestRuleExtraction();
    int TestRuleExtractionInitial();
    int TestRuleExtractionLen();

    bool RunTest();

private:
    HieroExtractor extractor;
    std::vector<Sentence> english;
    std::vector<Sentence> japan;
    std::vector<Alignment> align;
};

}

#endif

