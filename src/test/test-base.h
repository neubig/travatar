#ifndef TEST_BASE__
#define TEST_BASE__

#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

namespace travatar {

class TestBase {

public:

    TestBase() : passed_(false) { }
    virtual ~TestBase() { }

    // RunTest must be implemented by any test, and returns true if all
    // tests were passed
    virtual bool RunTest() = 0;

protected:

    bool passed_;

    template<class T>
    int CheckVector(const std::vector<T> & exp, const std::vector<T> & act) {
        int ok = 1;
        for(int i = 0; i < (int)max(exp.size(), act.size()); i++) {
            if(i >= (int)exp.size() || 
               i >= (int)act.size() || 
               exp[i] != act[i]) {
               
                ok = 0;
                std::cout << "exp["<<i<<"] != act["<<i<<"] (";
                if(i >= (int)exp.size()) std::cout << "NULL";
                else std::cout << exp[i];
                std::cout <<" != ";
                if(i >= (int)act.size()) std::cout << "NULL"; 
                else std::cout << act[i];
                std::cout << ")" << std::endl;
            }
        }
        return ok;
    }

    template<class T>
    int CheckAlmostVector(const std::vector<T> & exp,
                          const std::vector<T> & act) {
        int ok = 1;
        for(int i = 0; i < (int)max(exp.size(), act.size()); i++) {
            if(i >= (int)exp.size() || 
               i >= (int)act.size() || 
               abs(exp[i] - act[i]) > 0.01) {
               
                ok = 0;
                std::cout << "exp["<<i<<"] != act["<<i<<"] (";
                if(i >= (int)exp.size()) std::cout << "NULL";
                else std::cout << exp[i];
                std::cout <<" != ";
                if(i >= (int)act.size()) std::cout << "NULL"; 
                else std::cout << act[i];
                std::cout << ")" << std::endl;
            }
        }
        return ok;
    }

    int CheckAlmost(double exp, double act) {
        if(abs(exp - act) > 0.01) {
            std::cout << "CheckAlmost: " << exp << " != " << act << endl;
            return 0;
        }
        return 1;
    }

    int CheckString(const std::string & exp, const std::string & act) {
        if(exp != act) {
            cerr << "CheckString failed" << endl << "exp: '"<<exp<<"'"
                 <<endl<<"act: '"<<act<<"'" <<endl;
            for(int i = 0; i < (int)min(exp.length(), act.length()); i++)
                if(exp[i] != act[i])
                    cerr << "exp[" << i << "] '" << exp[i] << "' != act["<<i<<"] '"<<act[i]<<"'" <<endl;
            return 0;
        }
        return 1;
    }

};

}

#endif
