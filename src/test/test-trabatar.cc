#include <iostream>
#include "test-trabatar-runner.h"
#include "test-base.h"

using namespace std;
using namespace trabatar;

int main() {
    // Initialize all the tests
    vector<TestBase*> tests;
    tests.push_back(new TestTrabatarRunner());
    // Run all the tests
    int number_passed = 0;
    for(int i = 0; i < (int)tests.size(); i++)
        if(tests[i]->RunTest())
            number_passed++;
    // Check whether all were passed or not
    if(number_passed == (int)tests.size()) {
        cout << "**** passed ****" << endl;
    } else {
        cout << "**** FAILED!!! ****" << endl;
    }
    // Deallocate
    for(int i = 0; i < (int)tests.size(); i++)
        delete tests[i]; 
}
