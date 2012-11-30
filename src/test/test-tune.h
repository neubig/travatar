#ifndef TEST_TUNE_H__
#define TEST_TUNE_H__

#include "test-base.h"
#include <travatar/tune-greedy-mert.h>

namespace travatar {

class TestTune : public TestBase {

public:

    TestTune() { }
    ~TestTune() { }

    int TestCalculatePotentialGain() {
        TuneGreedyMert mert;
        // Create the weights
        SparseMap weights; weights[Dict::WID("c")] = 1;
        // Create the examples
        vector<TuneGreedyMert::ExamplePair> examps;
        SparseMap x1; x1[Dict::WID("a")] = 1; x1[Dict::WID("b")] = 1;
        examps.push_back(make_pair(x1, 1.0));
        SparseMap x2; x2[Dict::WID("a")] = 1; x2[Dict::WID("c")] = 1;
        examps.push_back(make_pair(x2, 0.5));
        SparseMap x3; x3[Dict::WID("a")] = 1; x3[Dict::WID("d")] = 1;
        examps.push_back(make_pair(x3, 0.0));
        // Find the expected and actual potential gain
        SparseMap gain_act = mert.CalculatePotentialGain(examps, weights);
        // Both b and c are different between x1 and x2, and should be active
        SparseMap gain_exp;
        gain_exp[Dict::WID("b")] = 0.5; gain_exp[Dict::WID("c")] = 0.5;
        return CheckMap(gain_exp, gain_act);
    }

    int TestCalculateConvexHullOne() {
        TuneGreedyMert mert;
        // Create the examples
        vector<TuneGreedyMert::ExamplePair> exps(3);
        exps[0].first[Dict::WID("val")] = 1;
        exps[0].first[Dict::WID("slope")] = -1;
        exps[0].second = 0.1;
        exps[1].first[Dict::WID("val")] = 3;
        exps[1].first[Dict::WID("slope")] = 1;
        exps[1].second = 0.2;
        exps[2].first[Dict::WID("val")] = 1;
        exps[2].first[Dict::WID("slope")] = 1;
        exps[2].second = 0.3;
        // Create the weights and the gradient
        SparseMap weights, gradient;
        weights[Dict::WID("val")] = 1;
        gradient[Dict::WID("slope")] = 1;
        // Here lines 0 and 1 should form the convex hull with an intersection at -1
        vector<TuneGreedyMert::ScoredSpan> hull_exp, hull_act;
        hull_act = mert.CalculateConvexHull(exps, weights, gradient);
        hull_exp.push_back(make_pair(make_pair(-DBL_MAX, -1.0), 0.1));
        hull_exp.push_back(make_pair(make_pair(-1.0, DBL_MAX), 0.2));  
        return CheckVector(hull_exp, hull_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestCalculatePotentialGain()" << endl; if(TestCalculatePotentialGain()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalculateConvexHullOne()" << endl; if(TestCalculateConvexHullOne()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTune Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

};

}

#endif
