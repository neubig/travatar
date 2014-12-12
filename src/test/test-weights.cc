#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/weights.h>
#include <travatar/weights-perceptron.h>
#include <travatar/weights-average-perceptron.h>
#include <travatar/dict.h>
#include <travatar/check-equal.h>

using namespace std;
using namespace travatar;

// ****** The tests *******
BOOST_AUTO_TEST_SUITE(weights)

BOOST_AUTO_TEST_CASE(TestPerceptronUpdate) {
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
    BOOST_CHECK(CheckMap(weights_exp.GetFinal(), weights_act.GetFinal()));
}

BOOST_AUTO_TEST_CASE(TestAvgPerceptronUpdate) {
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
    BOOST_CHECK(CheckMap(weights_exp.GetFinal(), weights_act.GetFinal()));
}

BOOST_AUTO_TEST_SUITE_END()
