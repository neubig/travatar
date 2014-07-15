#ifndef TEST_TUNE_H__
#define TEST_TUNE_H__

#include "test-base.h"
#include <travatar/tune-mert.h>
#include <travatar/tune-xeval.h>
#include <travatar/tune-greedy-mert.h>
#include <travatar/tuning-example-nbest.h>
#include <travatar/tuning-example-forest.h>

#include <boost/shared_ptr.hpp>

namespace travatar {

class TestTune : public TestBase {
public:
    TestTune();
    ~TestTune();

    int TestCalculatePotentialGain();
    int TestCalculateModelHypothesis();
    int TestCalculateConvexHull();
    int TestLineSearch();
    int TestLatticeHull();
    int TestForestHull();
    int TestMultipleForests();
    int TestForestUnk();
    int TestTuneXbleu();
    int TestScaleXbleu();
    int TestBigScaleXbleu();
    int TestBigScaleXbleup1();
    int TestL2Xbleu();
    int TestEntXbleu();

    bool RunTest();

private:
    int valid, slopeid;
    vector<boost::shared_ptr<TuningExample> > examp_set;
    boost::shared_ptr<HyperGraph> forest, forest2, forest2c, forest2d;
    SparseMap weights, gradient;
    TuningExampleNbest examp_nbest;

    Sentence GetQuotedWords(const std::string & str);
    HyperGraph * CreateForestTwo(bool use_c, bool use_d);
};

}

#endif
