#ifndef TEST_EVAL_MEASURE_H__
#define TEST_EVAL_MEASURE_H__

#include "test-base.h"
#include <travatar/eval-measure.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/eval-measure-interp.h>

namespace travatar {

class TestEvalMeasure : public TestBase {

public:

    TestEvalMeasure() { }
    ~TestEvalMeasure() { }

    int TestBleuScore() {
        EvalMeasureBleu eval_measure_bleu(4, 1, EvalMeasureBleu::SENTENCE);
        string ref = "taro met hanako", sys = "the taro met the hanako";
        Sentence ref_sent = Dict::ParseWords(ref), sys_sent = Dict::ParseWords(sys);
        // System is longer than ref, so there is no brevity penalty
        // Precision for BLEU+1 is ((3/5)*(2/5)*(1/4)*(1/3))^(1/4)
        double bleu_exp = exp(log((3.0/5.0) * (2.0/5.0) * (1.0/4.0) * (1.0/3.0))/4);
        double bleu_act = eval_measure_bleu.CalculateStats(ref_sent, sys_sent)->ConvertToScore();
        return CheckEqual(bleu_exp, bleu_act);
    }

    int TestInterpScore() {
        shared_ptr<EvalMeasure>
            bleu(EvalMeasure::CreateMeasureFromString("bleu:smooth=1")),
            ribes(EvalMeasure::CreateMeasureFromString("ribes")),
            interp(EvalMeasure::CreateMeasureFromString("interp:0.4|bleu:smooth=1|0.6|ribes"));
        string ref1 = "taro met hanako", sys1 = "the taro met the hanako";
        Sentence ref1_sent = Dict::ParseWords(ref1), sys1_sent = Dict::ParseWords(sys1);
        string ref2 = "he was the smallest man", sys2 = "the smallest man he was";
        Sentence ref2_sent = Dict::ParseWords(ref1), sys2_sent = Dict::ParseWords(sys1);
        // Eval stats pointer
        EvalStatsPtr bs1 = bleu->CalculateStats(ref1_sent, sys1_sent),
                     bs2 = bleu->CalculateStats(ref2_sent, sys2_sent),
                     rs1 = ribes->CalculateStats(ref1_sent, sys1_sent),
                     rs2 = ribes->CalculateStats(ref2_sent, sys2_sent),
                     is1 = interp->CalculateStats(ref1_sent, sys1_sent),
                     is2 = interp->CalculateStats(ref2_sent, sys2_sent);
        // Add the scores together
        double bleu_act = bs1->Plus(*bs2)->ConvertToScore();
        double ribes_act = rs1->Plus(*rs2)->ConvertToScore();
        double interp_exp = bleu_act*0.4+ribes_act*0.6;
        double interp_act = is1->Plus(*is2)->ConvertToScore();
        // Check that values are almost the same
        return CheckAlmost(interp_exp, interp_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestBleuScore()" << endl; if(TestBleuScore()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestInterpScore()" << endl; if(TestInterpScore()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestEvalMeasure Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

};

}

#endif
