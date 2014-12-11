#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/tokenizer-penn.h>

using namespace std;
using namespace boost;
using namespace travatar;

// ****** The fixture *******
struct TestTokenizer {

    TestTokenizer() { }
    ~TestTokenizer() { }

    TokenizerPenn tokenizer_penn_;

};

// ****** The tests *******
BOOST_AUTO_TEST_SUITE(tokenizer)

BOOST_FIXTURE_TEST_CASE(TestPenn, TestTokenizer) {
    string in = "\"Oh, no,\" she's said. i.e. \"our $400 blender, it can't handle something this hard!\"";
    string exp = "`` Oh , no , '' she 's said . i.e. `` our $ 400 blender , it ca n't handle something this hard ! ''";
    string act = tokenizer_penn_.Tokenize(in);
    BOOST_CHECK_EQUAL(exp, act);
}

BOOST_AUTO_TEST_SUITE_END()
