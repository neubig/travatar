#ifndef TEST_WEIGHTS_H__
#define TEST_WEIGHTS_H__

#include "test-base.h"
#include <travatar/weights.h>
#include <travatar/weights-perceptron.h>

namespace travatar {

class TestWeights : public TestBase {

public:

    TestWeights() { }
    ~TestWeights() { }

    int TestPerceptronUpdate() {
        WeightsPerceptron weights_act, weights_exp;
        // Set the initial weights
        weights_act.SetCurrent(Dict::WID("a"), 0.5);
        weights_act.SetCurrent(Dict::WID("b"), 0.5);
        // Set the values of the examples
        SparseMap oracle, sys_d, sys_f;
        oracle[Dict::WID("c")] = 0.5;
        oracle[Dict::WID("e")] = 0.5;
        sys_d[Dict::WID("d")] = 0.5;
        sys_d[Dict::WID("e")] = 0.5;
        sys_f[Dict::WID("f")] = 0.5;
        // The expected weights should be the difference between the examples
        weights_exp.SetCurrent(Dict::WID("a"), 0.5);
        weights_exp.SetCurrent(Dict::WID("b"), 0.5);
        weights_exp.SetCurrent(Dict::WID("c"), 0.5);
        weights_exp.SetCurrent(Dict::WID("d"), -0.5);
        // In the first update, sys_d's score is higher than the oracle
        weights_act.Update(oracle, 0.5, 0, sys_d, 1.0, 0.5);
        // In the second update, sys_f's score is lower -> no update
        weights_act.Update(oracle, 0.5, 0, sys_f, 0.3, 0.5);
        return CheckMap(weights_exp.GetFinal(), weights_act.GetFinal());
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestPerceptronUpdate()" << endl; if(TestPerceptronUpdate()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestWeights Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

};

}

#endif
