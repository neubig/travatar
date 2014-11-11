#include "test-weights.h"

#include <travatar/weights.h>
#include <travatar/weights-perceptron.h>
#include <travatar/weights-average-perceptron.h>
#include <travatar/dict.h>
#include <travatar/check-equal.h>

using namespace std;

namespace travatar {

TestWeights::TestWeights() { }

TestWeights::~TestWeights() { }

int TestWeights::TestPerceptronUpdate() {
    WeightsPerceptron weights_act, weights_exp;
    // Set the initial weights
    weights_act.SetCurrent(Dict::WID("a"), 0.5);
    weights_act.SetCurrent(Dict::WID("b"), 0.5);
    // Set the values of the examples
    SparseVector oracle, sys_d, sys_f;
    oracle.Add(Dict::WID("c"),0.5);
    oracle.Add(Dict::WID("e"),0.5);
    sys_d.Add(Dict::WID("d"),0.5);
    sys_d.Add(Dict::WID("e"),0.5);
    sys_f.Add(Dict::WID("f"),0.5);
    // The expected weights should be the difference between the examples
    weights_exp.SetCurrent(Dict::WID("a"), 0.5);
    weights_exp.SetCurrent(Dict::WID("b"), 0.5);
    weights_exp.SetCurrent(Dict::WID("c"), 0.5);
    weights_exp.SetCurrent(Dict::WID("d"), -0.5);
    // In the first update, sys_d's score is higher than the oracle
    weights_act.Update(oracle, 0.5, 1.0, sys_d, 1.0, 0.5);
    // In the second update, sys_f's score is lower -> no update
    weights_act.Update(oracle, 0.5, 1.0, sys_f, 0.3, 0.5);
    return CheckMap(weights_exp.GetFinal(), weights_act.GetFinal());
}

int TestWeights::TestAvgPerceptronUpdate() {
    WeightsAveragePerceptron weights_act, weights_exp;
    // Set the initial weights
    weights_act.SetCurrent(Dict::WID("a"), 0.5);
    weights_act.SetCurrent(Dict::WID("b"), 0.5);
    // Set the values of the examples
    SparseVector oracle, sys_d, sys_f;
    oracle.Add(Dict::WID("c"),0.5);
    oracle.Add(Dict::WID("e"),0.5);
    sys_d.Add(Dict::WID("d"),0.5);
    sys_d.Add(Dict::WID("e"),0.5);
    sys_f.Add(Dict::WID("f"),0.5);
    // In the first update, sys_d's score is higher than the oracle
    weights_act.Update(oracle, 0.5, 1.0, sys_d, 1.0, 0.5);
    // In the second update, sys_f's score is also higher
    weights_act.Update(oracle, 0.5, 1.0, sys_f, 0.7, 0.5);
    // The expected weights should be the average of when we subtracted sys_d and sys_f
    weights_exp.SetCurrent(Dict::WID("a"), 0.5);
    weights_exp.SetCurrent(Dict::WID("b"), 0.5);
    weights_exp.SetCurrent(Dict::WID("c"), 0.75);
    weights_exp.SetCurrent(Dict::WID("d"), -0.5);
    weights_exp.SetCurrent(Dict::WID("e"), 0.25);
    weights_exp.SetCurrent(Dict::WID("f"), -0.25);
    return CheckMap(weights_exp.GetFinal(), weights_act.GetFinal());
}

bool TestWeights::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestPerceptronUpdate()" << endl; if(TestPerceptronUpdate()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestAvgPerceptronUpdate()" << endl; if(TestAvgPerceptronUpdate()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestWeights Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

