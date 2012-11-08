#include <iostream>
#include "test-alignment.h"
#include "test-travatar-runner.h"
#include "test-hyper-graph.h"
#include "test-tree-io.h"
#include "test-rule-extractor.h"
#include "test-io-util.h"
#include "test-lookup-table.h"
#include "test-graph-transformer.h"
#include "test-binarizer.h"
#include "test-base.h"
#include "test-weights.h"
#include "test-eval-measure.h"
#include "test-tune.h"

using namespace std;
using namespace travatar;

int main() {
    // Initialize all the tests
    vector<TestBase*> tests;
    tests.push_back(new TestTreeIO());
    tests.push_back(new TestAlignment());
    tests.push_back(new TestIOUtil());
    tests.push_back(new TestHyperGraph());
    tests.push_back(new TestRuleExtractor());
    tests.push_back(new TestTravatarRunner());
    tests.push_back(new TestLookupTable());
    tests.push_back(new TestGraphTransformer());
    tests.push_back(new TestBinarizer());
    tests.push_back(new TestWeights());
    tests.push_back(new TestEvalMeasure());
    tests.push_back(new TestTune());
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
