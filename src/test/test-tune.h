#ifndef TEST_TUNE_H__
#define TEST_TUNE_H__

#include "test-base.h"
#include <travatar/tune-mert.h>
#include <travatar/tune-greedy-mert.h>
#include <travatar/tuning-example-nbest.h>
#include <travatar/tuning-example-forest.h>

namespace travatar {

class TestTune : public TestBase {

public:

    Sentence GetQuotedWords(const std::string & str) {
        Sentence words, syms;
        Dict::ParseQuotedWords(str, words, syms);
        return words;
    }

    TestTune() {
        valid = Dict::WID("val");
        slopeid = Dict::WID("slope");
        // Create the examples
        SparseMap feat10, feat11, feat12, feat20, feat21, feat22;
        shared_ptr<TuningExample> examps1(new TuningExampleNbest()), examps2(new TuningExampleNbest());
        feat10[valid] = 1; feat10[slopeid] = -1; ((TuningExampleNbest&)*examps1).AddHypothesis(feat10, EvalStatsPtr(new EvalStatsAverage(0.2, 1)));
        feat11[valid] = 3; feat11[slopeid] = 1;  ((TuningExampleNbest&)*examps1).AddHypothesis(feat11, EvalStatsPtr(new EvalStatsAverage(0.1, 1)));
        feat12[valid] = 1; feat12[slopeid] = 1;  ((TuningExampleNbest&)*examps1).AddHypothesis(feat12, EvalStatsPtr(new EvalStatsAverage(0.3, 1)));
        feat20[valid] = 7; feat20[slopeid] = -1; ((TuningExampleNbest&)*examps2).AddHypothesis(feat20, EvalStatsPtr(new EvalStatsAverage(0.1, 1)));
        feat21[valid] = 3; feat21[slopeid] = 1;  ((TuningExampleNbest&)*examps2).AddHypothesis(feat21, EvalStatsPtr(new EvalStatsAverage(0.2, 1)));
        feat22[valid] = 6; feat22[slopeid] = 0;  ((TuningExampleNbest&)*examps2).AddHypothesis(feat22, EvalStatsPtr(new EvalStatsAverage(0.3, 1)));
        examp_set.push_back(examps1); examp_set.push_back(examps2);
        weights[valid] = 1;
        gradient[slopeid] = 1;
                
        forest.reset(new HyperGraph);
        {
        // Two nodes
        HyperNode* n0 = new HyperNode(Dict::WID("A"), make_pair(0,1)); forest->AddNode(n0);
        HyperNode* n1 = new HyperNode(Dict::WID("B"), make_pair(0,1)); forest->AddNode(n1);
        // And two edges between each node
        SparseMap f01; f01[valid] = 1; f01[slopeid] = -1;
        HyperEdge* e01 = new HyperEdge(n0); e01->SetTrgWords(GetQuotedWords("x0 \"a\""));
        e01->SetFeatures(f01); e01->AddTail(n1); n0->AddEdge(e01); forest->AddEdge(e01);
        SparseMap f02; f02[valid] = 3; f02[slopeid] =  1;
        HyperEdge* e02 = new HyperEdge(n0); e02->SetTrgWords(GetQuotedWords("\"b\" x0"));
        e02->SetFeatures(f02); e02->AddTail(n1); n0->AddEdge(e02); forest->AddEdge(e02);
        SparseMap f11; f11[valid] = 1; f11[slopeid] = -1;
        HyperEdge* e11 = new HyperEdge(n1); e11->SetTrgWords(GetQuotedWords("\"c\""));
        e11->SetFeatures(f11);                   n1->AddEdge(e11); forest->AddEdge(e11);
        SparseMap f12; f12[valid] = 1; f12[slopeid] =  1;
        HyperEdge* e12 = new HyperEdge(n1); e12->SetTrgWords(GetQuotedWords("\"d\""));
        e12->SetFeatures(f12);                   n1->AddEdge(e12); forest->AddEdge(e12);         
        }

        forest2.reset(CreateForestTwo(true, true));
        forest2c.reset(CreateForestTwo(true, false));
        forest2d.reset(CreateForestTwo(false, true));
        // forest2a.reset(new HyperGraph);
        // forest2b.reset(new HyperGraph);
        // {
        // // Four nodes
        // HyperNode* n0 = new HyperNode(Dict::WID("A"), make_pair(0,1)); forest2->AddNode(n0);
        // HyperNode* n1 = new HyperNode(Dict::WID("B"), make_pair(0,1)); forest2->AddNode(n1);
        // HyperNode* n2 = new HyperNode(Dict::WID("C"), make_pair(0,1)); forest2->AddNode(n2);
        // HyperNode* n3 = new HyperNode(Dict::WID("D"), make_pair(0,1)); forest2->AddNode(n3);
        // // And edges for each node
        // SparseMap f01; f01[valid] = 1; f01[slopeid] = 0;
        // HyperEdge* e01 = new HyperEdge(n0); e01->SetTrgWords(GetQuotedWords("x0 x1"));
        // e01->SetFeatures(f01); e01->AddTail(n1); e01->AddTail(n2); n0->AddEdge(e01); forest2->AddEdge(e01);
        // SparseMap f02; f02[valid] = 1; f02[slopeid] =  2;
        // HyperEdge* e02 = new HyperEdge(n0); e02->SetTrgWords(GetQuotedWords("x1 x0"));
        // e02->SetFeatures(f02); e02->AddTail(n1); e02->AddTail(n3); n0->AddEdge(e02); forest2->AddEdge(e02);
        // SparseMap f11; f11[valid] = 1; f11[slopeid] = -1;
        // HyperEdge* e11 = new HyperEdge(n1); e11->SetTrgWords(GetQuotedWords("\"c\""));
        // e11->SetFeatures(f11); n1->AddEdge(e11); forest2->AddEdge(e11);
        // SparseMap f12; f12[valid] = 1; f12[slopeid] =  1;
        // HyperEdge* e12 = new HyperEdge(n1); e12->SetTrgWords(GetQuotedWords("\"d\""));
        // e12->SetFeatures(f12); n1->AddEdge(e12); forest2->AddEdge(e12);         
        // SparseMap f21; f21[valid] = 0; f21[slopeid] =  -1;
        // HyperEdge* e21 = new HyperEdge(n2); e21->SetTrgWords(GetQuotedWords("\"a\""));
        // e21->SetFeatures(f21); n2->AddEdge(e21); forest2->AddEdge(e21);
        // SparseMap f31; f31[valid] = 2; f31[slopeid] =  -1;
        // HyperEdge* e31 = new HyperEdge(n3); e31->SetTrgWords(GetQuotedWords("\"b\""));
        // e31->SetFeatures(f31); n3->AddEdge(e31); forest2->AddEdge(e31);
        // }
    }
    ~TestTune() { }

    // Create a forest using c and/or d
    HyperGraph * CreateForestTwo(bool use_c, bool use_d) {
        HyperGraph * ret = new HyperGraph;
        // Four nodes
        HyperNode* n0 = new HyperNode(Dict::WID("A"), make_pair(0,1)); ret->AddNode(n0);
        HyperNode* n1 = new HyperNode(Dict::WID("B"), make_pair(0,1)); ret->AddNode(n1);
        HyperNode *n2 = NULL, *n3 = NULL;
        if(use_c) {
            n2 = new HyperNode(Dict::WID("C"), make_pair(0,1)); ret->AddNode(n2);
        }
        if(use_d) {
            n3 = new HyperNode(Dict::WID("D"), make_pair(0,1)); ret->AddNode(n3);
        }
        // And edges for each node
        if(use_c) {
            SparseMap f01; f01[valid] = 1; f01[slopeid] = 0;
            HyperEdge* e01 = new HyperEdge(n0); e01->SetTrgWords(GetQuotedWords("x0 x1"));
            e01->SetFeatures(f01); e01->AddTail(n1); e01->AddTail(n2); n0->AddEdge(e01); ret->AddEdge(e01);
        }
        if(use_d) {
            SparseMap f02; f02[valid] = 1; f02[slopeid] =  2;
            HyperEdge* e02 = new HyperEdge(n0); e02->SetTrgWords(GetQuotedWords("x1 x0"));
            e02->SetFeatures(f02); e02->AddTail(n1); e02->AddTail(n3); n0->AddEdge(e02); ret->AddEdge(e02);
        }
        SparseMap f11; f11[valid] = 1; f11[slopeid] = -1;
        HyperEdge* e11 = new HyperEdge(n1); e11->SetTrgWords(GetQuotedWords("\"c\""));
        e11->SetFeatures(f11); n1->AddEdge(e11); ret->AddEdge(e11);
        SparseMap f12; f12[valid] = 1; f12[slopeid] =  1;
        HyperEdge* e12 = new HyperEdge(n1); e12->SetTrgWords(GetQuotedWords("\"d\""));
        e12->SetFeatures(f12); n1->AddEdge(e12); ret->AddEdge(e12);         
        if(use_c) {
            SparseMap f21; f21[valid] = 0; f21[slopeid] =  -1;
            HyperEdge* e21 = new HyperEdge(n2); e21->SetTrgWords(GetQuotedWords("\"a\""));
            e21->SetFeatures(f21); n2->AddEdge(e21); ret->AddEdge(e21);
        }
        if(use_d) {
            SparseMap f31; f31[valid] = 2; f31[slopeid] =  -1;
            HyperEdge* e31 = new HyperEdge(n3); e31->SetTrgWords(GetQuotedWords("\"b\""));
            e31->SetFeatures(f31); n3->AddEdge(e31); ret->AddEdge(e31);
        }
        return ret;
    }

    int TestCalculatePotentialGain() {
        TuneGreedyMert mert;
        // Create the weights
        SparseMap weights; weights[Dict::WID("c")] = 1;
        // Create the examples
        TuningExampleNbest examps;
        SparseMap x1; x1[Dict::WID("a")] = 1; x1[Dict::WID("b")] = 1;
        examps.AddHypothesis(x1, EvalStatsPtr(new EvalStatsAverage(1.0, 1)));
        SparseMap x2; x2[Dict::WID("a")] = 1; x2[Dict::WID("c")] = 1;
        examps.AddHypothesis(x2, EvalStatsPtr(new EvalStatsAverage(0.5, 1)));
        SparseMap x3; x3[Dict::WID("a")] = 1; x3[Dict::WID("d")] = 1;
        examps.AddHypothesis(x3, EvalStatsPtr(new EvalStatsAverage(0.0, 1)));
        // Find the expected and actual potential gain
        SparseMap gain_act = examps.CalculatePotentialGain(weights);
        // Both b and c are different between x1 and x2, and should be active
        SparseMap gain_exp;
        gain_exp[Dict::WID("b")] = 0.5; gain_exp[Dict::WID("c")] = 0.5;
        return CheckMap(gain_exp, gain_act);
    }

    int TestCalculateConvexHull() {
        TuneGreedyMert mert;
        // Here lines 0 and 1 should form the convex hull with an intersection at -1
        ConvexHull hull1_exp, hull1_act;
        hull1_act = examp_set[0]->CalculateConvexHull(weights, gradient);
        hull1_exp.push_back(make_pair(make_pair(-DBL_MAX, -1.0), EvalStatsPtr(new EvalStatsAverage(0.2, 1))));
        hull1_exp.push_back(make_pair(make_pair(-1.0, DBL_MAX), EvalStatsPtr(new EvalStatsAverage(0.1, 1))));  
        ConvexHull hull2_exp, hull2_act;
        hull2_act = examp_set[1]->CalculateConvexHull(weights, gradient);
        hull2_exp.push_back(make_pair(make_pair(-DBL_MAX, 1.0), EvalStatsPtr(new EvalStatsAverage(0.1, 1))));
        hull2_exp.push_back(make_pair(make_pair(1.0, 3.0), EvalStatsPtr(new EvalStatsAverage(0.3, 1))));
        hull2_exp.push_back(make_pair(make_pair(3.0, DBL_MAX), EvalStatsPtr(new EvalStatsAverage(0.2, 1))));
        return CheckVector(hull1_exp, hull1_act) && CheckVector(hull2_exp, hull2_act);
    }

    int TestLineSearch() {
        // Here lines 0 and 1 should form the convex hull with an intersection at -1
        LineSearchResult exp_score1(2.0, EvalStatsPtr(new EvalStatsAverage(0.2, 2)),EvalStatsPtr(new EvalStatsAverage(0.4, 2)));
        LineSearchResult act_score1 = TuneMert::LineSearch(weights, gradient, examp_set);
        LineSearchResult exp_score2(-2.0, EvalStatsPtr(new EvalStatsAverage(0.2, 2)),EvalStatsPtr(new EvalStatsAverage(0.3, 2)));
        LineSearchResult act_score2 = TuneMert::LineSearch(weights, gradient, examp_set, make_pair(-DBL_MAX, 0.0));
        LineSearchResult exp_score3(-3.0, EvalStatsPtr(new EvalStatsAverage(0.2, 2)),EvalStatsPtr(new EvalStatsAverage(0.3, 2)));
        LineSearchResult act_score3 = TuneMert::LineSearch(weights, gradient, examp_set, make_pair(-DBL_MAX, -3.0));
        return 
            CheckAlmost(exp_score1.pos, act_score1.pos) &&
            CheckAlmost(exp_score1.before->ConvertToScore(), act_score1.before->ConvertToScore()) &&
            CheckAlmost(exp_score1.after->ConvertToScore(), act_score1.after->ConvertToScore()) &&
            CheckAlmost(exp_score3.pos, act_score3.pos) &&
            CheckAlmost(exp_score3.before->ConvertToScore(), act_score3.before->ConvertToScore()) &&
            CheckAlmost(exp_score3.after->ConvertToScore(), act_score3.after->ConvertToScore()) &&
            CheckAlmost(exp_score2.pos, act_score2.pos) &&
            CheckAlmost(exp_score2.before->ConvertToScore(), act_score2.before->ConvertToScore()) &&
            CheckAlmost(exp_score2.after->ConvertToScore(), act_score2.after->ConvertToScore());
    }

    int TestLatticeHull() {
        EvalMeasureBleu bleu(4, 1, EvalMeasureBleu::SENTENCE);
        Sentence ref = Dict::ParseWords("c a");
        TuningExampleForest tef(&bleu, ref, 1, 1);
        tef.AddHypothesis(forest);
        ConvexHull exp_hull, act_hull = tef.CalculateConvexHull(weights, gradient);
        // The hypotheses are
        // "c a" --> w=2, s=-2 (BLEU=2/2, 2/2)
        // "d a" --> w=2, s=0  (BLEU=1/2, 1/2)
        // "b c" --> w=4, s=0  (BLEU=1/2, 1/2)
        // "b d" --> w=4, s=2  (BLEU=0/2, 1/2)
        exp_hull.push_back(make_pair(make_pair(-DBL_MAX,-1.0),   EvalStatsPtr(new EvalStatsAverage(1.0, 1))));
        exp_hull.push_back(make_pair(make_pair(-1.0,-DBL_MIN),   EvalStatsPtr(new EvalStatsAverage(exp((log(0.5)*2)/4), 1))));
        exp_hull.push_back(make_pair(make_pair(-DBL_MIN,DBL_MIN),EvalStatsPtr(new EvalStatsAverage(0.0, 1))));
        exp_hull.push_back(make_pair(make_pair(DBL_MIN,DBL_MAX), EvalStatsPtr(new EvalStatsAverage(0.0, 1))));
        return CheckVector(exp_hull, act_hull);
    }

    int TestForestHull() {
        EvalMeasureBleu bleu(4, 1, EvalMeasureBleu::SENTENCE);
        Sentence ref = Dict::ParseWords("c a");
        TuningExampleForest tef(&bleu, ref, 2, 1);
        tef.AddHypothesis(forest2);
        tef.CalculatePotentialGain(weights);
        ConvexHull exp_hull, act_hull = tef.CalculateConvexHull(weights, gradient);
        // The hypotheses are
        // "c a" --> w=2, s=-2 (BLEU=2/2, 2/2)
        // "d a" --> w=2, s=0  (BLEU=1/2, 1/2)
        // "b c" --> w=4, s=0  (BLEU=1/2, 1/2)
        // "b d" --> w=4, s=2  (BLEU=0/2, 1/2)
        exp_hull.push_back(make_pair(make_pair(-DBL_MAX,-1.0),   EvalStatsPtr(new EvalStatsAverage(1.0))));
        exp_hull.push_back(make_pair(make_pair(-1.0,-DBL_MIN),   EvalStatsPtr(new EvalStatsAverage(exp((log(0.5)*2)/4)))));
        exp_hull.push_back(make_pair(make_pair(-DBL_MIN,DBL_MIN),EvalStatsPtr(new EvalStatsAverage(0.0))));
        exp_hull.push_back(make_pair(make_pair(DBL_MIN,DBL_MAX), EvalStatsPtr(new EvalStatsAverage(0.0))));
        return CheckVector(exp_hull, act_hull);
    }

    int TestMultipleForests() {
        EvalMeasureBleu bleu(4, 1, EvalMeasureBleu::SENTENCE);
        Sentence ref = Dict::ParseWords("c a");
        TuningExampleForest tef(&bleu, ref, 2, 1);
        tef.AddHypothesis(forest2c);
        tef.AddHypothesis(forest2d);
        tef.CalculatePotentialGain(weights);
        ConvexHull exp_hull, act_hull = tef.CalculateConvexHull(weights, gradient);
        // The hypotheses are
        // "c a" --> w=2, s=-2 (BLEU=2/2, 2/2)
        // "d a" --> w=2, s=0  (BLEU=1/2, 1/2)
        // "b c" --> w=4, s=0  (BLEU=1/2, 1/2)
        // "b d" --> w=4, s=2  (BLEU=0/2, 1/2)
        exp_hull.push_back(make_pair(make_pair(-DBL_MAX,-1.0),   EvalStatsPtr(new EvalStatsAverage(1.0))));
        exp_hull.push_back(make_pair(make_pair(-1.0,-DBL_MIN),   EvalStatsPtr(new EvalStatsAverage(exp((log(0.5)*2)/4)))));
        exp_hull.push_back(make_pair(make_pair(-DBL_MIN,DBL_MIN),EvalStatsPtr(new EvalStatsAverage(0.0))));
        exp_hull.push_back(make_pair(make_pair(DBL_MIN,DBL_MAX), EvalStatsPtr(new EvalStatsAverage(0.0))));
        return CheckVector(exp_hull, act_hull);
    }

    int TestForestUnk() {
        EvalMeasureBleu bleu(4, 1, EvalMeasureBleu::SENTENCE);
        shared_ptr<HyperGraph> rule_graph(new HyperGraph);
        Sentence exp_sent(1,Dict::WID("wordA")), act_sent;
        rule_graph->SetWords(exp_sent);
        HyperNode* n0 = new HyperNode(Dict::WID("A"), make_pair(0,1)); rule_graph->AddNode(n0);
        SparseMap f01; f01[valid] = -10; f01[slopeid] = 1;
        HyperEdge* e01 = new HyperEdge(n0); e01->SetTrgWords(GetQuotedWords("\"<unk>\""));
        e01->SetFeatures(f01); n0->AddEdge(e01); rule_graph->AddEdge(e01);
        HyperEdge* e02 = new HyperEdge(n0); e02->SetTrgWords(GetQuotedWords("\"hello\""));
        n0->AddEdge(e02); rule_graph->AddEdge(e02);
        TuningExampleForest tef(&bleu, exp_sent, 2, 1);
        tef.AddHypothesis(rule_graph);
        tef.CalculatePotentialGain(weights);
        // The check here should break
        ConvexHull act_hull = tef.CalculateConvexHull(weights, gradient);
        ConvexHull exp_hull;
        exp_hull.push_back(make_pair(make_pair(-DBL_MAX, 10.0), EvalStatsPtr(new EvalStatsAverage(0.0))));
        exp_hull.push_back(make_pair(make_pair(10.0, DBL_MAX),  EvalStatsPtr(new EvalStatsAverage(1.0))));
        return CheckVector(act_hull,exp_hull);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestCalculatePotentialGain()" << endl; if(TestCalculatePotentialGain()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateConvexHull()" << endl; if(TestCalculateConvexHull()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestLineSearch()" << endl; if(TestLineSearch()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestLatticeHull()" << endl; if(TestLatticeHull()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestForestHull()" << endl; if(TestForestHull()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestMultipleForests()" << endl; if(TestMultipleForests()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestForestUnk()" << endl; if(TestForestUnk()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTune Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

    int valid, slopeid;
    vector<shared_ptr<TuningExample> > examp_set;
    shared_ptr<HyperGraph> forest, forest2, forest2c, forest2d;
    SparseMap weights, gradient;

};

}

#endif
