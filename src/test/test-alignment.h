#ifndef TEST_ALIGNMENT_H__
#define TEST_ALIGNMENT_H__

#include "test-base.h"
#include <travatar/alignment.h>
#include <travatar/dict.h>
#include <boost/scoped_ptr.hpp>

namespace travatar {

class TestAlignment : public TestBase {

public:

    TestAlignment() {
        align.AddAlignment(MakePair(0,0));
        align.AddAlignment(MakePair(0,1));
        align.AddAlignment(MakePair(1,1));
        align.AddAlignment(MakePair(1,3));
    }
    ~TestAlignment() { }

    int TestSrcAlignments() {
        vector<set<int> > set_align, set_act;
        set_align[0].insert(0);
        set_align[0].insert(1);
        set_align[1].insert(1);
        set_align[1].insert(3);
        set_act = align.GetSrcAlignments();
        return CheckVector(set_align, set_act);
    }

    int TestReadWrite() {
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

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestReadWrite()" << endl; if(TestReadWrite()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestAlignment Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    Alignment align;

};

}

#endif
