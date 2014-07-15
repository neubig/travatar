#include "test-lookup-table-fsm.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

namespace travatar {

TestLookupTableFSM::TestLookupTableFSM() {
    // Load the rules
    ostringstream rule_oss;
    rule_oss << "\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
    rule_oss << "\"eat\" \"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 2
    rule_oss << "\"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 3
    rule_oss << "x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 4
    rule_oss << "\"eat\" x0:X @ X ||| x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 5
    rule_oss << "\"I\" x0:X \"two\" \"hamburgers\" @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 6
    rule_oss << "\"I\" x0:X \"two\" x1:X @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1:X \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 7
    rule_oss << "\"I\" @ X ||| \"watashi\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 8
    rule_oss << "\"eat\" @ X ||| \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 9
    rule_oss << "\"two\" @ X ||| \"futatsu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 10
    rule_oss << "\"hamburgers\" @ X ||| \"hanbaga\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 11
    istringstream rule_iss(rule_oss.str());
    lookup_fsm.reset(new LookupTableFSM);
    lookup_fsm->SetRootSymbol(Dict::WID("X"));
    lookup_fsm->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss));

    // Load the rules
    ostringstream rule_oss1, rule_oss2;
    rule_oss1 << "\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
    rule_oss1 << "\"eat\" \"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 2
    rule_oss2 << "\"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 3
    rule_oss2 << "x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 4
    rule_oss2 << "\"eat\" x0:X @ X ||| x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 5
    rule_oss2 << "\"I\" x0:X \"two\" \"hamburgers\" @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 6
    rule_oss2 << "\"I\" x0:X \"two\" x1:X @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1:X \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 7
    rule_oss2 << "\"I\" @ X ||| \"watashi\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 8
    rule_oss2 << "\"eat\" @ X ||| \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 9
    rule_oss2 << "\"two\" @ X ||| \"futatsu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 10
    rule_oss1 << "\"hamburgers\" @ X ||| \"hanbaga\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 11
    istringstream rule_iss1(rule_oss1.str());
    istringstream rule_iss2(rule_oss2.str());
    lookup_fsm_split.reset(new LookupTableFSM);
    lookup_fsm_split->SetRootSymbol(Dict::WID("X"));
    lookup_fsm_split->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss1));
    lookup_fsm_split->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss2));

    // Load the rules
    ostringstream rule_oss_extra;
    rule_oss_extra << "\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
    rule_oss_extra << "\"eat\" \"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 2
    rule_oss_extra << "\"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 3
    rule_oss_extra << "x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 4
    rule_oss_extra << "\"eat\" x0:X @ X ||| x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 5
    rule_oss_extra << "\"I\" x0:X \"two\" \"hamburgers\" @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 6
    rule_oss_extra << "\"I\" x0:X \"two\" x1:X @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1:X \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 7
    rule_oss_extra << "\"I\" @ X ||| \"watashi\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 8
    rule_oss_extra << "\"eat\" @ X ||| \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 9
    rule_oss_extra << "\"two\" @ X ||| \"futatsu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 10
    rule_oss_extra << "\"hamburgers\" @ X ||| \"hanbaga\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 11
    // rule_oss_extra << "x0:X @ BAD ||| x0:X @ BAD ||| Pegf=0.02 ppen=2.718" << endl; // 11
    rule_oss_extra << "\"two\" x0:DIFF @ X ||| \"watashi\" \"wa\" x0:DIFF @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
    istringstream rule_iss_extra(rule_oss_extra.str());
    lookup_fsm_extra.reset(new LookupTableFSM);
    lookup_fsm_extra->SetRootSymbol(Dict::WID("X"));
    lookup_fsm_extra->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_extra));
}

TestLookupTableFSM::~TestLookupTableFSM() { }

boost::shared_ptr<TranslationRuleHiero> TestLookupTableFSM::BuildRule(const string & src, const string & trg, const string & feat) {
    return boost::shared_ptr<TranslationRuleHiero>(new TranslationRuleHiero(
        src,
        Dict::ParseAnnotatedVector(trg),
        Dict::ParseFeatures(feat),
        Dict::ParseAnnotatedWords(src)
    ));
}

HyperGraph * TestLookupTableFSM::CreateExpectedGraph() {
    vector<HyperNode*> node(6);
    vector<HyperEdge*> edge(10);
    vector<boost::shared_ptr<TranslationRuleHiero> > rules(11);

    for (int i=0; i < (int)node.size(); ++i) {
        node[i] = new HyperNode;
        node[i]->SetSym(Dict::WID("X"));
    }
    for (int j=0; j < (int)edge.size(); ++j) edge[j] = new HyperEdge;

    // Transform into Hiero rule
    vector<string> word, target;
    rules[0] = BuildRule("\"I\" x0:X @ X", "\"watashi\" \"wa\" x0:X @ X", "Pegf=0.02 ppen=2.718");
    rules[1] = BuildRule("\"I\" x0:X \"two\" \"hamburgers\" @ X", "\"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0:X @ X", "Pegf=0.02 ppen=2.718");
    rules[2] = BuildRule("\"I\" @ X", "\"watashi\" @ X", "Pegf=0.02 ppen=2.718");
    rules[3] = BuildRule("\"I\" x0:X \"two\" x1:X @ X", "\"watashi\" \"wa\" \"futatsu\" \"no\" x1:X \"wo\" x0:X @ X", "Pegf=0.02 ppen=2.718");
    rules[4] = BuildRule("\"eat\" \"two\" x0:X @ X", "\"futatsu\" \"no\" x0:X \"wo\" \"taberu\" @ X", "Pegf=0.02 ppen=2.718");
    rules[5] = BuildRule("x0:X \"eat\" x1:X @ X", "x0:X \"wa\" x1:X \"wo\" \"taberu\" @ X", "Pegf=0.02 ppen=2.718");
    rules[6] = BuildRule("\"eat\" x0:X @ X", "x0:X \"wo\" \"taberu\" @ X", "Pegf=0.02 ppen=2.718");
    rules[7] = BuildRule("\"eat\" @ X", "\"taberu\" @ X", "Pegf=0.02 ppen=2.718");
    rules[8] = BuildRule("\"two\" x0:X @ X", "\"futatsu\" \"no\" x0:X @ X", "Pegf=0.02 ppen=2.718");
    rules[9] = BuildRule("\"two\" @ X", "\"futatsu\" @ X", "Pegf=0.02 ppen=2.718");
    rules[10] = BuildRule("\"hamburgers\" @ X", "\"hanbaga\" @ X", "Pegf=0.02 ppen=2.718");

    // Draw it. You will have an idea after you see the drawing.
    // X[0,4] -> X[0,1] ate X[2,4]
    edge[0]->SetHead(node[0]); 
    edge[0]->AddTail(node[1]); 
    edge[0]->AddTail(node[4]); 
    edge[0]->SetRule(rules[5].get(), rules[5]->GetFeatures());
    // X[3,4] -> hamburgers
    edge[1]->SetHead(node[5]);
    edge[1]->SetRule(rules[10].get(), rules[10]->GetFeatures());
    // X[2,4] -> two X[3,4]
    edge[2]->SetHead(node[4]); 
    edge[2]->AddTail(node[5]);
    edge[2]->SetRule(rules[8].get(), rules[8]->GetFeatures());
    // X[1,4] -> ate X[2,4]
    edge[3]->SetHead(node[3]); 
    edge[3]->AddTail(node[4]); 
    edge[3]->SetRule(rules[6].get(), rules[6]->GetFeatures());
    // X[1,4] -> ate two X[3,4]
    edge[4]->SetHead(node[3]); 
    edge[4]->AddTail(node[5]); 
    edge[4]->SetRule(rules[4].get(), rules[4]->GetFeatures());
    // X[1,2] -> ate
    edge[5]->SetHead(node[2]); 
    edge[5]->SetRule(rules[7].get(), rules[7]->GetFeatures());
    // X[0,4] -> i X[1,2] two X[3,4]
    edge[6]->SetHead(node[0]); 
    edge[6]->AddTail(node[2]);
    edge[6]->AddTail(node[5]); 
    edge[6]->SetRule(rules[3].get(), rules[3]->GetFeatures());
    // X[0,4] -> i X[1,2] two hamburgers
    edge[7]->SetHead(node[0]); 
    edge[7]->AddTail(node[2]);
    edge[7]->SetRule(rules[1].get(), rules[1]->GetFeatures());
    // X[0,4] -> i X[1,4]
    edge[8]->SetHead(node[0]); 
    edge[8]->AddTail(node[3]); 
    edge[8]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    // X[0,1] -> i
    edge[9]->SetHead(node[1]); 
    edge[9]->SetRule(rules[2].get(), rules[2]->GetFeatures());
   
    // 0: X[0,4]
    node[0]->SetSpan(pair<int,int>(0,4)); 
    node[0]->AddEdge(edge[0]); 
    node[0]->AddEdge(edge[6]); 
    node[0]->AddEdge(edge[7]); 
    node[0]->AddEdge(edge[8]); 
    // 1: X[0,1]
    node[1]->SetSpan(pair<int,int>(0,1)); 
    node[1]->AddEdge(edge[9]); 
    // 2: X[1,2]
    node[2]->SetSpan(pair<int,int>(1,2)); 
    node[2]->AddEdge(edge[5]); 
    // 3: X[1,4]
    node[3]->SetSpan(pair<int,int>(1,4)); 
    node[3]->AddEdge(edge[3]); 
    node[3]->AddEdge(edge[4]);
    // 4: X[2,4]
    node[4]->SetSpan(pair<int,int>(2,4)); 
    node[4]->AddEdge(edge[2]); 
    // 5: X[3,4]
    node[5]->SetSpan(pair<int,int>(3,4)); 
    node[5]->AddEdge(edge[1]); 

    HyperGraph* expected_graph = new HyperGraph;
    BOOST_FOREACH(HyperEdge* ed, edge) {
        expected_graph->AddEdge(ed);   
    }

    BOOST_FOREACH(HyperNode* nd, node) {
        expected_graph->AddNode(nd);
    }

    string inp = "I eat two hamburgers";
    Sentence c = Dict::ParseWords(inp);
    BOOST_FOREACH(WordId w_id, c) {
        expected_graph->AddWord(w_id);
    }
    return expected_graph;
}

bool TestLookupTableFSM::TestBuildRules(LookupTableFSM & lookup) {
    string inp = "I eat two hamburgers";
    Sentence c = Dict::ParseWords(inp);

    boost::shared_ptr<HyperGraph> input_graph(new HyperGraph);
    BOOST_FOREACH(WordId word, Dict::ParseWords(inp)) 
        input_graph->AddWord(word);

    HyperGraph* actual_graph = lookup.TransformGraph(*input_graph);

    HyperGraph* expected_graph = CreateExpectedGraph();

    bool ret = expected_graph->CheckMaybeEqual(*actual_graph);
    delete actual_graph;
    delete expected_graph;
    return ret;
}

bool TestLookupTableFSM::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestBuildRules(lookup_fsm)" << endl; if(TestBuildRules(*lookup_fsm)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBuildRules(lookup_fsm_split)" << endl; if(TestBuildRules(*lookup_fsm_split)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBuildRules(lookup_fsm_extra)" << endl; if(TestBuildRules(*lookup_fsm_extra)) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestLookupTableFSM Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

