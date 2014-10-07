#include "test-tokenizer.h"

#include <travatar/dict.h>
#include <travatar/check-equal.h>
#include <travatar/tree-io.h>
#include <travatar/hyper-graph.h>

#include <vector>

using namespace std;
using namespace boost;

namespace travatar {

TestTokenizer::TestTokenizer() { }
TestTokenizer::~TestTokenizer() { }

int TestTokenizer::TestPenn() {
    string in = "\"Oh, no,\" she's saying, \"our $400 blender can't handle something this hard!\"";
    string exp = "`` Oh , no , '' she 's saying , `` our $ 400 blender ca n't handle something this hard ! ''";
    string act = tokenizer_penn_.Tokenize(in);
    return CheckEqual(exp, act);
}

bool TestTokenizer::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestPenn()" << endl; if(TestPenn()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestTokenizer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

