#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/io-util.h>

using namespace std;
using namespace travatar;

// ****** The tests *******
BOOST_AUTO_TEST_SUITE(io_util)

BOOST_AUTO_TEST_CASE(TestTrim) {
    // Test to make sure that trimming whitespace works properly
    stringstream str;
    string line;
    str << " \nAAA";
    IoUtil::Trim(str, WHITE_SPACE);
    getline(str, line);
    int ret = (line == "AAA");
    if(!ret) cerr << "Expected AAA but got "<<line<<endl;
    BOOST_CHECK(ret);
}

BOOST_AUTO_TEST_CASE(TestReadUntil) {
    // Test to make sure that trimming whitespace works properly
    stringstream str;
    string s1, s2;
    str << "ABCD XYZ";
    s1 = IoUtil::ReadUntil(str, WHITE_SPACE);
    getline(str, s2);
    int ret = (s1 == "ABCD") && (s2 == " XYZ");
    if(!ret) 
        cerr << "s1==" << s1 <<", s2==" << s2 << endl;
    BOOST_CHECK(ret);
}

BOOST_AUTO_TEST_SUITE_END()
