#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/math-query.h>
#include <travatar/check-equal.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

BOOST_AUTO_TEST_SUITE(math_query)

BOOST_AUTO_TEST_CASE(TestReadQueryAdd) {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),0.1));
    vars.insert(make_pair(Dict::WID("B"),0.2));
    ostringstream oss;
    MathQuery mq ("0.5*(A+(B+A)/2.5)", vars);
    mq.Print(oss);
    BOOST_CHECK(CheckEqual(oss.str(),string("0.5*(0.1+(0.2+0.1)/2.5)"))); 
}

BOOST_AUTO_TEST_CASE(TestEvalQueryBasic) {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),1));
    vars.insert(make_pair(Dict::WID("B"),2));
    BOOST_CHECK(CheckAlmost(MathQuery::Evaluate(vars,"2*3+1-2+5/2.5"),7));
}

BOOST_AUTO_TEST_CASE(TestEvalQueryIntermediate) {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),1));
    vars.insert(make_pair(Dict::WID("B"),2));
    BOOST_CHECK(CheckAlmost(MathQuery::Evaluate(vars,"A*3+(B-2+5)/2.5"),5));
}

BOOST_AUTO_TEST_CASE(TestEvalQueryAdvance) {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),1));
    vars.insert(make_pair(Dict::WID("B"),2));
    vars.insert(make_pair(Dict::WID("C"),5));
    BOOST_CHECK(CheckAlmost(MathQuery::Evaluate(vars,"((A*B)+C-1)-5*2+((B-2+5)/2.5)*B"),0));

}
  
BOOST_AUTO_TEST_CASE(TestEvalQueryFailOpen) {
    std::map<WordId, double> vars;
    BOOST_CHECK_THROW(MathQuery::Evaluate(vars,"((1+2)"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TestEvalQueryFailClosed) {
    std::map<WordId, double> vars;
    BOOST_CHECK_THROW(MathQuery::Evaluate(vars,"(1+2))"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TestEvalQueryFailOperator) {
    std::map<WordId, double> vars;
    BOOST_CHECK_THROW(MathQuery::Evaluate(vars,"1 2"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TestEvalQueryFailOperand) {
    std::map<WordId, double> vars;
    BOOST_CHECK_THROW(MathQuery::Evaluate(vars,"1+"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TestEvalQueryFailVariable) {
    std::map<WordId, double> vars;
    BOOST_CHECK_THROW(MathQuery::Evaluate(vars,"A+1"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TestInvalidQuery) {
    std::map<WordId, double> vars;
    std::vector<string> tc;
    tc.push_back("1^2");
    tc.push_back("a+1");
    tc.push_back("((a+2)");
    tc.push_back("a");
    BOOST_FOREACH(string s, tc) {
        BOOST_CHECK_THROW(MathQuery::Evaluate(vars,s), std::runtime_error);
    }
}

BOOST_AUTO_TEST_SUITE_END()
