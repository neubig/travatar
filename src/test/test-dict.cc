#include "test-dict.h"
#include <travatar/util.h>
#include <boost/foreach.hpp>

using namespace std;

namespace travatar {

TestDict::TestDict() {}
TestDict::~TestDict() {}

int TestDict::TestParseWords() {
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
    return CheckVector(strs_exp, strs_act);
}

int TestDict::TestParseFeatures() {
    // Parse the vector
    SparseMap feat_act = Dict::ParseFeatures("a=1.5 b=2");
    SparseMap feat_exp;
    feat_exp[Dict::WID("a")] = 1.5;
    feat_exp[Dict::WID("b")] = 2;
    return CheckMap(feat_exp, feat_act);
}

bool TestDict::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestParseWords()" << endl; if(TestParseWords()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestParseFeatures()" << endl; if(TestParseFeatures()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestDict Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

