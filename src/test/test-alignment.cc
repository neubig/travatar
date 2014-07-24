#include "test-alignment.h"
#include <travatar/check-equal.h>

using namespace std;

namespace travatar {

TestAlignment::TestAlignment() {
    align.AddAlignment(make_pair(0,0));
    align.AddAlignment(make_pair(0,1));
    align.AddAlignment(make_pair(1,1));
    align.AddAlignment(make_pair(1,3));
}

TestAlignment::~TestAlignment() {}

int TestAlignment::TestSrcAlignments() {
    vector<set<int> > set_align, set_act;
    set_align[0].insert(0);
    set_align[0].insert(1);
    set_align[1].insert(1);
    set_align[1].insert(3);
    set_act = align.GetSrcAlignments();
    return CheckVector(set_align, set_act);
}

int TestAlignment::TestReadWrite() {
    string str = align.ToString();
    Alignment act = Alignment::FromString(str);
    int ret = 1;
    if(align != act) {
        cerr << "align '" << align.ToString() << 
            "' != act '"<<act.ToString()<<"'" <<endl;
        ret = 0;
    }
    return ret;
}

bool TestAlignment::RunTest() {
    int done = 0, succeeded = 0;
    ++done;
    cout << "TestReadWrite()" << endl;
    if (TestReadWrite()) ++succeeded;
    else cout << "FAILED!!!" << endl;
    cout << "#### TestAlignment Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

