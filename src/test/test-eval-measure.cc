#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/dict.h>
#include <travatar/check-equal.h>
#include <travatar/eval-measure.h>
#include <travatar/eval-measure-loader.h>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace travatar;

struct TestEvalMeasure {

public:
    TestEvalMeasure() :
        eval_measure_bleu1_(EvalMeasureLoader::CreateMeasureFromString("bleu:order=1")),
        eval_measure_bleup1_(EvalMeasureLoader::CreateMeasureFromString("bleu:smooth=1")),
        eval_measure_bleup1r_(EvalMeasureLoader::CreateMeasureFromString("bleu:smooth=1,prec=0")),
        eval_measure_bleup1f_(EvalMeasureLoader::CreateMeasureFromString("bleu:smooth=1,prec=0.5")),
        eval_measure_bleup1a_(EvalMeasureLoader::CreateMeasureFromString("bleu:smooth=1,mean=arith")),
        eval_measure_ribes_(EvalMeasureLoader::CreateMeasureFromString("ribes")),
        eval_measure_ter_(EvalMeasureLoader::CreateMeasureFromString("ter")),
        eval_measure_wer_(EvalMeasureLoader::CreateMeasureFromString("wer")),
        eval_measure_pincbleu_(EvalMeasureLoader::CreateMeasureFromString("interp:0.5|bleu:factor=0,smooth=1|0.5|bleu:inverse=true,brev=false,factor=1,scope=sentence,mean=arith")),
        eval_measure_interp_(EvalMeasureLoader::CreateMeasureFromString("interp:0.4|bleu:smooth=1|0.6|ribes")),
        eval_measure_adv_interp_(EvalMeasureLoader::CreateMeasureFromString("ainterp:A|bleu:smooth=1|B|ribes|2*A*B/(A+B)")),
        ref1_sent_(Dict::ParseWords("taro met hanako")),
        sys1_sent_(Dict::ParseWords("the taro met the hanako"))
    { }
    ~TestEvalMeasure() { }

    boost::shared_ptr<EvalMeasure> eval_measure_bleu1_, eval_measure_bleup1_, eval_measure_bleup1r_, eval_measure_bleup1f_, eval_measure_bleup1a_, eval_measure_ribes_, eval_measure_ter_, eval_measure_wer_, eval_measure_pincbleu_, eval_measure_interp_, eval_measure_adv_interp_;
    Sentence ref1_sent_, sys1_sent_;

};

// ****** The tests *******
BOOST_FIXTURE_TEST_SUITE(eval_measure, TestEvalMeasure)


BOOST_AUTO_TEST_CASE(TestBleuIO) {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_bleup1_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_bleup1_->ReadStats(str);
    BOOST_CHECK(exp_stats->Equals(*act_stats) && CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString()));
}

BOOST_AUTO_TEST_CASE(TestRibesIO) {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_ribes_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_ribes_->ReadStats(str);
    BOOST_CHECK(exp_stats->Equals(*act_stats) && CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString()));
}

BOOST_AUTO_TEST_CASE(TestTerIO) {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_ter_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_ter_->ReadStats(str);
    BOOST_CHECK(exp_stats->Equals(*act_stats) && CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString()));
}

BOOST_AUTO_TEST_CASE(TestInterpIO) {
    vector<double> vals;
    EvalStatsPtr exp_stats(eval_measure_interp_->CalculateStats(ref1_sent_,sys1_sent_));
    string str = exp_stats->WriteStats();
    EvalStatsPtr act_stats = eval_measure_interp_->ReadStats(str);
    BOOST_CHECK(CheckEqual(exp_stats->ConvertToString(), act_stats->ConvertToString()) && exp_stats->Equals(*act_stats));
}

BOOST_AUTO_TEST_CASE(TestBleu1) {
    double bleu_exp = 3.0/5.0;
    double bleu_act = eval_measure_bleu1_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    BOOST_CHECK(CheckAlmost(bleu_exp,bleu_act));
}

BOOST_AUTO_TEST_CASE(TestBleuScore) {
    // System is longer than ref, so there is no brevity penalty
    // Precision for BLEU+1 is ((3/5)*(2/5)*(1/4)*(1/3))^(1/4)
    double bleu_exp = exp(log((3.0/5.0) * (2.0/5.0) * (1.0/4.0) * (1.0/3.0))/4);
    double bleu_act = eval_measure_bleup1_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    BOOST_CHECK(CheckAlmost(bleu_exp, bleu_act));
}

BOOST_AUTO_TEST_CASE(TestBleuRecall) {
    // System is longer than ref, so there is no brevity penalty
    // Recall for BLEU+1 is ((3/3)*(2/3)*(1/2)*(1/1))^(1/4)
    double bleu_exp = exp(log((3.0/3.0) * (2.0/3.0) * (1.0/2.0) * (1.0/1.0))/4);
    double bleu_act = eval_measure_bleup1r_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    BOOST_CHECK(CheckAlmost(bleu_exp, bleu_act));
}

BOOST_AUTO_TEST_CASE(TestBleuFmeas) {
    // System is longer than ref, so there is no brevity penalty
    // Recall for BLEU+1 is ((3/3)*(2/3)*(1/2)*(1/1))^(1/4)
    double p1 = 3.0/5.0, r1 = 3.0/3.0, f1=2*p1*r1/(p1+r1);
    double p2 = 2.0/5.0, r2 = 2.0/3.0, f2=2*p2*r2/(p2+r2);
    double p3 = 1.0/4.0, r3 = 1.0/2.0, f3=2*p3*r3/(p3+r3);
    double p4 = 1.0/3.0, r4 = 1.0/1.0, f4=2*p4*r4/(p4+r4);
    double bleu_exp = exp(log(f1 * f2 * f3 * f4)/4);
    double bleu_act = eval_measure_bleup1f_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    BOOST_CHECK(CheckAlmost(bleu_exp, bleu_act));
}

BOOST_AUTO_TEST_CASE(TestBleuArith) {
    // System is longer than ref, so there is no brevity penalty
    // Precision for BLEU+1 is ((3/5)*(2/5)*(1/4)*(1/3))^(1/4)
    double bleu_exp = ((3.0/5.0) + (2.0/5.0) + (1.0/4.0) + (1.0/3.0))/4;
    double bleu_act = eval_measure_bleup1a_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    BOOST_CHECK(CheckAlmost(bleu_exp, bleu_act));
}

BOOST_AUTO_TEST_CASE(TestWerScore) {
    double wer_exp = 2.0/3.0;
    double wer_act = eval_measure_wer_->CalculateStats(ref1_sent_, sys1_sent_)->ConvertToScore();
    BOOST_CHECK(CheckAlmost(wer_exp, wer_act));
}

BOOST_AUTO_TEST_CASE(TestInterpScore) {
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
    BOOST_CHECK(CheckAlmost(interp_exp, interp_act));
}

BOOST_AUTO_TEST_CASE(TestAdvInterpScore) {
    string ref2 = "he was the smallest man", sys2 = "the smallest man he was";
    Sentence ref2_sent = Dict::ParseWords(ref2), sys2_sent = Dict::ParseWords(sys2);
    // Eval stats pointer
    EvalStatsPtr bs1 = eval_measure_bleup1_->CalculateStats(ref1_sent_, sys1_sent_),
                 bs2 = eval_measure_bleup1_->CalculateStats(ref2_sent, sys2_sent),
                 rs1 = eval_measure_ribes_->CalculateStats(ref1_sent_, sys1_sent_),
                 rs2 = eval_measure_ribes_->CalculateStats(ref2_sent, sys2_sent),
                 is1 = eval_measure_adv_interp_->CalculateStats(ref1_sent_, sys1_sent_),
                 is2 = eval_measure_adv_interp_->CalculateStats(ref2_sent, sys2_sent);
    // Add the scores together
    double bleu_act = bs1->Plus(*bs2)->ConvertToScore();
    double ribes_act = rs1->Plus(*rs2)->ConvertToScore();
    double interp_exp = 2*bleu_act*ribes_act/(bleu_act+ribes_act);
    double interp_act = is1->Plus(*is2)->ConvertToScore();
    // Check that values are almost the same
    BOOST_CHECK(CheckAlmost(interp_exp, interp_act));
}

BOOST_AUTO_TEST_CASE(TestPincScore) {
    vector<Sentence> ref_sent = Dict::ParseWordVector("taro met hanako |COL| taro went to hanako 's house");
    vector<Sentence> sys_sent = Dict::ParseWordVector("the taro met the hanako |COL| the taro met the hanako");
    EvalStatsPtr act_stat = eval_measure_pincbleu_ -> CalculateCachedStats(ref_sent, sys_sent);
    double bleu_exp = exp(log((3.0/5.0) * (2.0/5.0) * (1.0/4.0) * (1.0/3.0))/4);
    double pinc_exp = 1.0 - 0.25 * ((2.0/5.0) + (0.0/4.0) + (0.0/3.0) + (0.0/2.0));
    double exp_score = 0.5 * bleu_exp + 0.5 * pinc_exp;
    double act_score = act_stat->ConvertToScore();
    BOOST_CHECK(CheckAlmost(exp_score, act_score)); 
}

BOOST_AUTO_TEST_SUITE_END()
