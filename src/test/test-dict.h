#ifndef TEST_DICT_H__
#define TEST_DICT_H__

#include "test-base.h"
#include <travatar/dict.h>

namespace travatar {

class TestDict : public TestBase {

public:

    TestDict() { }
    ~TestDict() { }

    int TestParseWords() {
        // Parse the vector
        vector<WordId> ids = Dict::ParseWords("a b a c");
        vector<string> strs_act;
        BOOST_FOREACH(WordId wid, ids)
            strs_act.push_back(Dict::WSym(wid));
        // Create the vector
        strs_HERE
        return CheckVector(strs_exp, strs_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestParseWords()" << endl; if(TestParseWords()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestDict Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

};

}

#endif
