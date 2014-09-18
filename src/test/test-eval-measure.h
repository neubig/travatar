#ifndef TEST_EVAL_MEASURE_H__
#define TEST_EVAL_MEASURE_H__

#include "test-base.h"
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

class TestEvalMeasure : public TestBase {

public:
    TestEvalMeasure();
    ~TestEvalMeasure();

    int TestBleuIO();
    int TestRibesIO();
    int TestTerIO();
    int TestInterpIO();
    int TestBleuScore();
    int TestBleuRecall();
    int TestBleuFmeas();
    int TestBleuArith();
    int TestWerScore();
    int TestInterpScore();
    int TestPincScore();
    bool RunTest();

protected:
    boost::shared_ptr<EvalMeasure> eval_measure_bleup1_, eval_measure_bleup1r_, eval_measure_bleup1f_, eval_measure_bleup1a_, eval_measure_ribes_, eval_measure_ter_, eval_measure_wer_, eval_measure_pincbleu_, eval_measure_interp_;
    Sentence ref1_sent_, sys1_sent_;

};

}

#endif
