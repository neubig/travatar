
#include "test-math-query.h"
#include <travatar/check-equal.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>

using namespace std;

namespace travatar {

TestMathQuery::TestMathQuery() {
    
}

bool TestMathQuery::TestReadQueryAdd() {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),0.1));
    vars.insert(make_pair(Dict::WID("B"),0.2));
    ostringstream oss;
    MathQuery mq ("0.5*(A+(B+A)/2.5)", vars);
    mq.Print(oss);
    return CheckEqual(oss.str(),string("0.5*(0.1+(0.2+0.1)/2.5)")); 
}

bool TestMathQuery::TestEvalQueryBasic() {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),1));
    vars.insert(make_pair(Dict::WID("B"),2));
    return CheckAlmost(MathQuery::Evaluate(vars,"2*3+1-2+5/2.5"),7);
}

bool TestMathQuery::TestEvalQueryIntermediate() {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),1));
    vars.insert(make_pair(Dict::WID("B"),2));
    return CheckAlmost(MathQuery::Evaluate(vars,"A*3+(B-2+5)/2.5"),5);
}

bool TestMathQuery::TestEvalQueryAdvance() {
    std::map<WordId,double> vars;
    vars.insert(make_pair(Dict::WID("A"),1));
    vars.insert(make_pair(Dict::WID("B"),2));
    vars.insert(make_pair(Dict::WID("C"),5));
    return CheckAlmost(MathQuery::Evaluate(vars,"((A*B)+C-1)-5*2+((B-2+5)/2.5)*B"),0);

}
  
bool TestMathQuery::TestEvalQueryFailOpen() {
    std::map<WordId, double> vars;
    try {
        MathQuery::Evaluate(vars,"((1+2)");
    } catch(std::runtime_error e) {
        return 1;
    }
    return 0;
}

bool TestMathQuery::TestEvalQueryFailClosed() {
    std::map<WordId, double> vars;
    try {
        MathQuery::Evaluate(vars,"(1+2))");
    } catch(std::runtime_error e) {
        return 1;
    }
    return 0;
}

bool TestMathQuery::TestEvalQueryFailOperator() {
    std::map<WordId, double> vars;
    try {
        MathQuery::Evaluate(vars,"1 2");
    } catch(std::runtime_error e) {
        return 1;
    }  
    return 0;
}

bool TestMathQuery::TestEvalQueryFailOperand() {
    std::map<WordId, double> vars;
    try {
        MathQuery::Evaluate(vars,"1+");
    } catch(std::runtime_error e) {
        return 1;
    }
    return 0;
}

bool TestMathQuery::TestEvalQueryFailVariable() {
    std::map<WordId, double> vars;
    try {
        MathQuery::Evaluate(vars,"A+1");
    } catch(std::runtime_error e) {
        return 1;
    }
    return 0;
}

bool TestMathQuery::TestInvalidQuery() {
    std::map<WordId, double> vars;
    std::vector<string> tc;
    tc.push_back("1^2");
    tc.push_back("a+1");
    tc.push_back("((a+2)");
    tc.push_back("a");
    BOOST_FOREACH(string s, tc) {
        try {
            MathQuery::Evaluate(vars, s);
            cerr << "Expecting error on : '" << s << "', but not error!" << endl;
            return 0;
        } catch (std::runtime_error e) {
        }
    }
    return 1;
}

bool TestMathQuery::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestReadQuery()" << endl; if(TestReadQueryAdd()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryBasic()" << endl; if(TestEvalQueryBasic()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryIntermediate()" << endl; if(TestEvalQueryIntermediate()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryFailOpen()" << endl; if(TestEvalQueryFailOpen()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryFailClosed()" << endl; if(TestEvalQueryFailClosed()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryFailOperator()" << endl; if(TestEvalQueryFailOperator()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryFailOperand()" << endl; if(TestEvalQueryFailOperand()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryFailVariable()" << endl; if(TestEvalQueryFailVariable()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestEvalQueryAdvance()" << endl; if(TestEvalQueryAdvance()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestInvalidQuery()" << endl; if(TestInvalidQuery()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestMathQuery Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
} 
}
