#include <iostream>
#include "test-alignment.h"
#include "test-travatar-runner.h"
#include "test-tree-io.h"
#include "test-rule-extractor.h"
#include "test-io-util.h"
#include "test-base.h"

using namespace std;
using namespace travatar;

int main() {
    // Initialize all the tests
    vector<TestBase*> tests;
    tests.push_back(new TestAlignment());
    tests.push_back(new TestIOUtil());
    tests.push_back(new TestTreeIO());
    tests.push_back(new TestRuleExtractor());
    tests.push_back(new TestTravatarRunner());
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
