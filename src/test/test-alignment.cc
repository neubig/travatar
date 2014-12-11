#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/check-equal.h>
#include <travatar/alignment.h>

using namespace std;
using namespace travatar;

// ****** The fixture *******
struct TestAlignment {

    TestAlignment() {
        align.AddAlignment(make_pair(0,0));
        align.AddAlignment(make_pair(0,1));
        align.AddAlignment(make_pair(1,1));
        align.AddAlignment(make_pair(1,3));
    }
    ~TestAlignment() { }

    Alignment align;
};


// ****** The tests *******
BOOST_FIXTURE_TEST_SUITE(alignment, TestAlignment)

BOOST_AUTO_TEST_CASE(TestSrcAlignments) {
    vector<set<int> > set_align(2), set_act;
    set_align[0].insert(0);
    set_align[0].insert(1);
    set_align[1].insert(1);
    set_align[1].insert(3);
    set_act = align.GetSrcAlignments();
    BOOST_CHECK(CheckVector(set_align, set_act));
}

BOOST_AUTO_TEST_CASE(TestReadWrite) {
    string str = align.ToString();
    Alignment act = Alignment::FromString(str);
    if(align != act) {
        cerr << "align '" << align.ToString() << 
            "' != act '"<<act.ToString()<<"'" <<endl;
        BOOST_FAIL("Failed");
    }
}

BOOST_AUTO_TEST_SUITE_END()
