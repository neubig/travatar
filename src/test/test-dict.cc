#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/check-equal.h>
#include <vector>
#include <boost/foreach.hpp>

using namespace std;
using namespace travatar;

BOOST_AUTO_TEST_SUITE(dict)

BOOST_AUTO_TEST_CASE(TestParseWords) {
    // Parse the vector
    vector<WordId> ids = Dict::ParseWords("a b a c");
    vector<string> strs_act;
    BOOST_FOREACH(WordId wid, ids)
        strs_act.push_back(Dict::WSym(wid));
    vector<string> strs_exp(4);
    strs_exp[0] = "a";
    strs_exp[1] = "b";
    strs_exp[2] = "a";
    strs_exp[3] = "c";
    BOOST_CHECK(CheckVector(strs_exp, strs_act));
}

BOOST_AUTO_TEST_CASE(TestParseSparseMap) {
    // Parse the vector
    SparseMap feat_act = Dict::ParseSparseMap("a=1.5 b=2");
    SparseMap feat_exp;
    feat_exp[Dict::WID("a")] = 1.5;
    feat_exp[Dict::WID("b")] = 2;
    BOOST_CHECK(CheckMap(feat_exp, feat_act));
}

BOOST_AUTO_TEST_CASE(TestParseSparseMapError) {
    // Parser the vector
    try {
        SparseMap error_map = Dict::ParseSparseMap("a=0 b=1 a=0");
        BOOST_CHECK(false);
    } catch (std::runtime_error& e) {
        BOOST_CHECK(true);
    }
}

BOOST_AUTO_TEST_SUITE_END()
