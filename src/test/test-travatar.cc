#define BOOST_TEST_MODULE "Travatar Tests"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// #include "test-alignment.h"
// #include "test-binarizer.h"
// #include "test-caser.h"
// #include "test-dict.h"
// #include "test-eval-measure.h"
// #include "test-graph-transformer.h"
// #include "test-hiero.h"
// #include "test-hyper-graph.h"
// #include "test-io-util.h"
// #include "test-lm-composer.h"
// #include "test-lookup-table.h"
// #include "test-lookup-table-fsm.h"
// #include "test-rule-extractor.h"
// #include "test-tokenizer.h"
// #include "test-tree-io.h"
// #include "test-trimmer.h"
// #include "test-tune.h"
// #include "test-weights.h"
// #include "test-math-query.h"
// 
// #include "test-base.h"
// 
// #include <iostream>
// 
// using namespace std;
// using namespace travatar;
// 
// int main() {
//     // Initialize all the tests
//     vector<TestBase*> tests;
//     tests.push_back(new TestAlignment());
//     tests.push_back(new TestBinarizer());
//     tests.push_back(new TestCaser());
//     tests.push_back(new TestDict());
//     tests.push_back(new TestEvalMeasure());
//     tests.push_back(new TestGraphTransformer());
//     tests.push_back(new TestHiero());
//     tests.push_back(new TestHyperGraph());
//     tests.push_back(new TestIOUtil());
//     tests.push_back(new TestLMComposer());
//     tests.push_back(new TestLookupTable());
//     tests.push_back(new TestLookupTableFSM());
//     tests.push_back(new TestRuleExtractor());
//     tests.push_back(new TestTokenizer());
//     tests.push_back(new TestTreeIO());
//     tests.push_back(new TestTrimmer());
//     tests.push_back(new TestTune());
//     tests.push_back(new TestWeights());
//     tests.push_back(new TestMathQuery());
//     // Run all the tests
//     int number_passed = 0;
//     for(int i = 0; i < (int)tests.size(); i++)
//         if(tests[i]->RunTest())
//             number_passed++;
//     // Check whether all were passed or not
//     if(number_passed == (int)tests.size()) {
//         cout << "**** passed ****" << endl;
//     } else {
//         cout << "**** FAILED!!! ****" << endl;
//     }
//     // Deallocate
//     for(int i = 0; i < (int)tests.size(); i++)
//         delete tests[i]; 
// }
