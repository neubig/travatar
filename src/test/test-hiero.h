#ifndef TEST_HIERO_H__  
#define TEST_HIERO_H__

#include "test-base.h"
#include <travatar/hiero-extractor.h>
#include <travatar/alignment.h>
#include <travatar/sentence.h>
#include <vector>
#include <set>
#include <string>

namespace travatar {

class TestHiero : public TestBase {

public:
    TestHiero();
    ~TestHiero();
    
    int PhraseTest(std::vector<std::set<std::string> > & exp);
    int TestPhraseExtraction();
    int TestPhraseExtractionLimit();
    int RuleTest(std::vector<std::set<std::string> > & exp);
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

