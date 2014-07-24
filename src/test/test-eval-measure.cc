#include "test-eval-measure.h"
#include <travatar/dict.h>
#include <travatar/check-equal.h>

using namespace std;

namespace travatar {

TestEvalMeasure::TestEvalMeasure() :
    eval_measure_bleup1_(EvalMeasure::CreateMeasureFromString("bleu:smooth=1")),
    eval_measure_bleup1r_(EvalMeasure::CreateMeasureFromString("bleu:smooth=1,prec=0")),
    eval_measure_bleup1f_(EvalMeasure::CreateMeasureFromString("bleu:smooth=1,prec=0.5")),
    eval_measure_bleup1a_(EvalMeasure::CreateMeasureFromString("bleu:smooth=1,mean=arith")),
    eval_measure_ribes_(EvalMeasure::CreateMeasureFromString("ribes")),
    eval_measure_ter_(EvalMeasure::CreateMeasureFromString("ter")),
    eval_measure_wer_(EvalMeasure::CreateMeasureFromString("wer")),
    eval_measure_interp_(EvalMeasure::CreateMeasureFromString("interp:0.4|bleu:smooth=1|0.6|ribes")),
    ref1_sent_(Dict::ParseWords("taro met hanako")),
    sys1_sent_(Dict::ParseWords("the taro met the hanako"))
{ }

TestEvalMeasure::~TestEvalMeasure() { }

int TestEvalMeasure::TestBleuIO() {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_bleup1_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_bleup1_->ReadStats(str);
    return exp_stats->Equals(*act_stats) && CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString());
}

int TestEvalMeasure::TestRibesIO() {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_ribes_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_ribes_->ReadStats(str);
    return exp_stats->Equals(*act_stats) && CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString());
}

int TestEvalMeasure::TestTerIO() {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_ter_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_ter_->ReadStats(str);
    return exp_stats->Equals(*act_stats) && CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString());
}

int TestEvalMeasure::TestInterpIO() {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_interp_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_interp_->ReadStats(str);
    return CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString()) && exp_stats->Equals(*act_stats);
}

int TestEvalMeasure::TestBleuScore() {
    // System is longer than ref, so there is no brevity penalty
    // Precision for BLEU+1 is ((3/5)*(2/5)*(1/4)*(1/3))^(1/4)
    double bleu_exp = exp(log((3.0/5.0) * (2.0/5.0) * (1.0/4.0) * (1.0/3.0))/4);
    double bleu_act = eval_measure_bleup1_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    return CheckAlmost(bleu_exp, bleu_act);
}

int TestEvalMeasure::TestBleuRecall() {
    // System is longer than ref, so there is no brevity penalty
    // Recall for BLEU+1 is ((3/3)*(2/3)*(1/2)*(1/1))^(1/4)
    double bleu_exp = exp(log((3.0/3.0) * (2.0/3.0) * (1.0/2.0) * (1.0/1.0))/4);
    double bleu_act = eval_measure_bleup1r_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    return CheckAlmost(bleu_exp, bleu_act);
}

int TestEvalMeasure::TestBleuFmeas() {
    // System is longer than ref, so there is no brevity penalty
    // Recall for BLEU+1 is ((3/3)*(2/3)*(1/2)*(1/1))^(1/4)
    double p1 = 3.0/5.0, r1 = 3.0/3.0, f1=2*p1*r1/(p1+r1);
    double p2 = 2.0/5.0, r2 = 2.0/3.0, f2=2*p2*r2/(p2+r2);
    double p3 = 1.0/4.0, r3 = 1.0/2.0, f3=2*p3*r3/(p3+r3);
    double p4 = 1.0/3.0, r4 = 1.0/1.0, f4=2*p4*r4/(p4+r4);
    double bleu_exp = exp(log(f1 * f2 * f3 * f4)/4);
    double bleu_act = eval_measure_bleup1f_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    return CheckAlmost(bleu_exp, bleu_act);
}

int TestEvalMeasure::TestBleuArith() {
    // System is longer than ref, so there is no brevity penalty
    // Precision for BLEU+1 is ((3/5)*(2/5)*(1/4)*(1/3))^(1/4)
    double bleu_exp = ((3.0/5.0) + (2.0/5.0) + (1.0/4.0) + (1.0/3.0))/4;
    double bleu_act = eval_measure_bleup1a_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    return CheckAlmost(bleu_exp, bleu_act);
}

int TestEvalMeasure::TestWerScore() {
    double wer_exp = 2.0/3.0;
    double wer_act = eval_measure_wer_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    return CheckAlmost(wer_exp, wer_act);
}

int TestEvalMeasure::TestInterpScore() {
    string ref2 = "he was the smallest man", sys2 = "the smallest man he was";
    Sentence ref2_sent = Dict::ParseWords(ref2), sys2_sent = Dict::ParseWords(sys2);
    // Eval stats pointer
    EvalStatsPtr bs1 = eval_measure_bleup1_->CalculateStats(ref1_sent_, sys1_sent_),
                 bs2 = eval_measure_bleup1_->CalculateStats(ref2_sent, sys2_sent),
                 rs1 = eval_measure_ribes_->CalculateStats(ref1_sent_, sys1_sent_),
                 rs2 = eval_measure_ribes_->CalculateStats(ref2_sent, sys2_sent),
                 is1 = eval_measure_interp_->CalculateStats(ref1_sent_, sys1_sent_),
                 is2 = eval_measure_interp_->CalculateStats(ref2_sent, sys2_sent);
    // Add the scores together
    double bleu_act = bs1->Plus(*bs2)->ConvertToScore();
    double ribes_act = rs1->Plus(*rs2)->ConvertToScore();
    double interp_exp = bleu_act*0.4+ribes_act*0.6;
    double interp_act = is1->Plus(*is2)->ConvertToScore();
    // Check that values are almost the same
    return CheckAlmost(interp_exp, interp_act);
}

bool TestEvalMeasure::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestBleuIO()" << endl; if(TestBleuIO()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRibesIO()" << endl; if(TestRibesIO()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestTerIO()" << endl; if(TestTerIO()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestInterpIO()" << endl; if(TestInterpIO()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBleuScore()" << endl; if(TestBleuScore()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBleuRecall()" << endl; if(TestBleuRecall()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBleuFmeas()" << endl; if(TestBleuFmeas()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBleuArith()" << endl; if(TestBleuArith()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestWerScore()" << endl; if(TestWerScore()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestInterpScore()" << endl; if(TestInterpScore()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestEvalMeasure Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

