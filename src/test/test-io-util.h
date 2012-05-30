#ifndef TEST_IO_UTIL_H__
#define TEST_IO_UTIL_H__

#include "test-base.h"
#include <travatar/io-util.h>

namespace travatar {

class TestIOUtil : public TestBase {

public:

    TestIOUtil() { }
    ~TestIOUtil() { }

    int TestTrim() {
        // Test to make sure that trimming whitespace works properly
        stringstream str;
        string line;
        str << " \nAAA";
        Trim(str, WHITE_SPACE);
        getline(str, line);
        int ret = (line == "AAA");
        if(!ret) cerr << "Expected AAA but got "<<line<<endl;
        return ret;
    }

    int TestReadUntil() {
        // Test to make sure that trimming whitespace works properly
        stringstream str;
        string s1, s2;
        str << "ABCD XYZ";
        s1 = ReadUntil(str, WHITE_SPACE);
        getline(str, s2);
        int ret = (s1 == "ABCD") && (s2 == " XYZ");
        if(!ret) 
            cerr << "s1==" << s1 <<", s2==" << s2 << endl;
        return ret;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestTrim()" << endl; if(TestTrim()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestReadUntil()" << endl; if(TestReadUntil()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestIOUtil Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
