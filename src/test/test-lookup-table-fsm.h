#ifndef TEST_LOOKUP_TABLE_FSM_H__
#define TEST_LOOKUP_TABLE_FSM_H__

#include "test-base.h"
#include <travatar/lookup-table-fsm.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace boost;

namespace travatar {

class TestLookupTableFSM : public TestBase {
public:

    TestLookupTableFSM() {
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "\"I\" x0 ||| \"watashi\" \"wa\" x0 ||| Pegf=0.02 ppen=2.718" << endl; // 1
        rule_oss << "\"eat\" \"two\" x0 ||| \"futatsu\" \"no\" x0 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl; // 2
        rule_oss << "\"two\" x0 ||| \"futatsu\" \"no\" x0 ||| Pegf=0.02 ppen=2.718" << endl; // 3
        rule_oss << "x0 \"eat\" x1 ||| x0 \"wa\" x1 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl; // 4
        rule_oss << "\"eat\" x0 ||| x0 \"wo\" \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl; // 5
        rule_oss << "\"I\" x0 \"two\" \"hamburgers\" ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0 ||| Pegf=0.02 ppen=2.718" << endl; // 6
        rule_oss << "\"I\" x0 \"two\" x1 ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1 \"wo\" x0 ||| Pegf=0.02 ppen=2.718" << endl; // 7
        rule_oss << "\"I\" ||| \"watashi\" ||| Pegf=0.02 ppen=2.718" << endl; // 8
        rule_oss << "\"eat\" ||| \"taberu\" ||| Pegf=0.02 ppen=2.718" << endl; // 9
        rule_oss << "\"two\" ||| \"futatsu\" ||| Pegf=0.02 ppen=2.718" << endl; // 10
        rule_oss << "\"hamburgers\" ||| \"hanbaga\" ||| Pegf=0.02 ppen=2.718" << endl; // 11

        istringstream rule_iss(rule_oss.str());
        lookup_fsm.reset(LookupTableFSM::ReadFromRuleTable(rule_iss));

        ostringstream rule_oss_gen;
        rule_oss_gen << "x0 \"a\" x1 ||| x0 \"a\" x1 ||| pgef=0.02" << endl;
        rule_oss_gen << "x0 x1 \"a\" ||| x0 x1 \"a\" ||| pgef=0.02" << endl;
        rule_oss_gen << "\"a\" x1 x0 \"b\" ||| x0 \"a\" \"b\" x1 ||| pgef=0.02" << endl;
        rule_oss_gen << "x0 x1 x2 ||| x0 x1 x2 ||| pgef=0.02" << endl;
        rule_oss_gen << "x0 \"a\" ||| x0 \"a\" ||| pgef=0.02" << endl;
        rule_oss_gen << "\"a\" x0 ||| \"a\" x0 ||| pgef=0.02" << endl;
        rule_oss_gen << "x0 x1 ||| x0 x1 ||| pgef=0.02" << endl;
        istringstream rule_iss_gen (rule_oss_gen.str());
        lookup_fsm_general.reset(LookupTableFSM::ReadFromRuleTable(rule_iss_gen));
    }

    bool TestBuildRules(LookupTableFSM & lookup) {
        string inp = "I eat two hamburgers";
        Sentence c = Dict::ParseWords(inp);

        HyperGraph* input_graph = new HyperGraph;
        BOOST_FOREACH(WordId word, Dict::ParseWords(inp)) 
            input_graph->AddWord(word);

        HyperGraph* actual_graph = lookup.TransformGraph(*input_graph);

        HyperNode* node[10];
        HyperEdge* edge[21];

        for (int i=0; i < 10; ++i) node[i] = new HyperNode;
        for (int j=0; j < 21; ++j) edge[j] = new HyperEdge;

        TranslationRuleHiero* rules[11];
        for (int i=0; i < 11; ++i) {
            rules[i] = new TranslationRuleHiero;
        }

        // Transform into Hiero rule
        vector<string> word, target;
        algorithm::split(word, "\"I\" x0", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" x0", is_any_of(" "));
        rules[0] = (LookupTableHiero::BuildRule(rules[0], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718")));
        
        algorithm::split(word, "\"I\" x0 \"two\" \"hamburgers\"", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0", is_any_of(" "));
        rules[1] = LookupTableHiero::BuildRule(rules[1], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"I\"", is_any_of(" ")); algorithm::split(target, "\"watashi\"", is_any_of(" "));
        rules[2] = LookupTableHiero::BuildRule(rules[2], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"I\" x0 \"two\" x1", is_any_of(" ")); algorithm::split(target, "\"watashi\" \"wa\" \"futatsu\" \"no\" x1 \"wo\" x0", is_any_of(" "));
        rules[3] = LookupTableHiero::BuildRule(rules[3], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
       
        algorithm::split(word, "\"eat\" \"two\" x0", is_any_of(" ")); algorithm::split(target, "\"futatsu\" \"no\" x0 \"wo\" \"taberu\"", is_any_of(" "));
        rules[4] = LookupTableHiero::BuildRule(rules[4], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "x0 \"eat\" x1", is_any_of(" ")); algorithm::split(target, "x0 \"wa\" x1 \"wo\" \"taberu\"", is_any_of(" "));
        rules[5] = LookupTableHiero::BuildRule(rules[5], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"eat\" x0", is_any_of(" ")); algorithm::split(target, "x0 \"wo\" \"taberu\"", is_any_of(" "));
        rules[6] = LookupTableHiero::BuildRule(rules[6], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"eat\"", is_any_of(" ")); algorithm::split(target, "\"taberu\"", is_any_of(" "));
        rules[7] = LookupTableHiero::BuildRule(rules[7], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"two\" x0", is_any_of(" ")); algorithm::split(target, "\"futatsu\" \"no\" x0", is_any_of(" "));
        rules[8] = LookupTableHiero::BuildRule(rules[8], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"two\"", is_any_of(" ")); algorithm::split(target, "\"futatsu\"", is_any_of(" "));
        rules[9] = LookupTableHiero::BuildRule(rules[9], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));
        
        algorithm::split(word, "\"hamburgers\"", is_any_of(" ")); algorithm::split(target, "\"hanbaga\"", is_any_of(" "));
        rules[10] = LookupTableHiero::BuildRule(rules[10], word, target, Dict::ParseFeatures("Pegf=0.02 ppen=2.718"));

        TranslationRuleHiero* glue_rule = NULL;

        // Draw it. You will have an idea after you see the drawing.
        edge[0]->SetHead(node[0]); 
        {
            edge[0]->AddTail(node[1]); 
            edge[0]->AddTail(node[8]); 
            edge[0]->SetRule(rules[5], rules[5]->GetFeatures());
        }
        edge[1]->SetHead(node[3]); 
        {
            edge[1]->AddTail(node[1]); 
            edge[1]->AddTail(node[7]); 
            edge[1]->SetRule(rules[5], rules[5]->GetFeatures());
        }
        edge[2]->SetHead(node[9]);
        {
            edge[2]->SetRule(rules[10], rules[10]->GetFeatures());
        } 
        edge[3]->SetHead(node[8]); 
        {
            edge[3]->AddTail(node[9]);
            edge[3]->SetRule(rules[8], rules[8]->GetFeatures());
        }
        edge[4]->SetHead(node[7]); 
        {
            edge[4]->SetRule(rules[9], rules[9]->GetFeatures());
        }
        edge[5]->SetHead(node[6]); 
        {
            edge[5]->AddTail(node[8]); 
            edge[5]->SetRule(rules[6], rules[6]->GetFeatures());
        }
        edge[6]->SetHead(node[5]); 
        { 
            edge[6]->AddTail(node[7]); 
            edge[6]->SetRule(rules[6], rules[6]->GetFeatures());
        }
        edge[7]->SetHead(node[6]); 
        {
            edge[7]->AddTail(node[9]); 
            edge[7]->SetRule(rules[4], rules[4]->GetFeatures());
        }
        edge[8]->SetHead(node[4]); 
        {
            edge[8]->SetRule(rules[7], rules[7]->GetFeatures());
        }
        edge[9]->SetHead(node[0]); 
        {
            edge[9]->AddTail(node[4]);
            edge[9]->AddTail(node[9]); 
            edge[9]->SetRule(rules[3], rules[3]->GetFeatures());
        }
        edge[10]->SetHead(node[0]); 
        {
            edge[10]->AddTail(node[4]);
            edge[10]->SetRule(rules[1], rules[1]->GetFeatures());
        }
        edge[11]->SetHead(node[0]); 
        {
            edge[11]->AddTail(node[6]); 
            edge[11]->SetRule(rules[0], rules[0]->GetFeatures());
        }
        edge[12]->SetHead(node[3]); 
        {
            edge[12]->AddTail(node[5]); 
            edge[12]->SetRule(rules[0], rules[0]->GetFeatures());
        }
        edge[13]->SetHead(node[2]); 
        {
            edge[13]->AddTail(node[4]);
            edge[13]->SetRule(rules[0], rules[0]->GetFeatures());
        }
        edge[14]->SetHead(node[1]); 
        {
            edge[14]->SetRule(rules[2], rules[2]->GetFeatures());
        }
        edge[15]->SetHead(node[2]); edge[15]->AddTail(node[1]); edge[15]->AddTail(node[4]); edge[15]->SetRule(glue_rule, glue_rule->GetFeatures());
        edge[16]->SetHead(node[3]); edge[16]->AddTail(node[1]); edge[16]->AddTail(node[5]); edge[16]->SetRule(glue_rule, glue_rule->GetFeatures());
        edge[17]->SetHead(node[3]); edge[17]->AddTail(node[2]); edge[17]->AddTail(node[7]); edge[17]->SetRule(glue_rule, glue_rule->GetFeatures());
        edge[18]->SetHead(node[0]); edge[18]->AddTail(node[1]); edge[18]->AddTail(node[6]); edge[18]->SetRule(glue_rule, glue_rule->GetFeatures());
        edge[19]->SetHead(node[0]); edge[19]->AddTail(node[2]); edge[19]->AddTail(node[8]); edge[19]->SetRule(glue_rule, glue_rule->GetFeatures());
        edge[20]->SetHead(node[0]); edge[20]->AddTail(node[3]); edge[20]->AddTail(node[9]); edge[20]->SetRule(glue_rule, glue_rule->GetFeatures());
       
        node[0]->SetSpan(pair<int,int>(0,4)); 
        {
            node[0]->AddEdge(edge[0]); 
            node[0]->AddEdge(edge[9]); 
            node[0]->AddEdge(edge[10]); 
            node[0]->AddEdge(edge[11]); 
            node[0]->AddEdge(edge[18]); 
            node[0]->AddEdge(edge[19]); 
            node[0]->AddEdge(edge[20]);
        }
        node[1]->SetSpan(pair<int,int>(0,1)); 
        {
            node[1]->AddEdge(edge[14]);
        }
        node[2]->SetSpan(pair<int,int>(0,2)); 
        {
            node[2]->AddEdge(edge[13]); 
            node[2]->AddEdge(edge[15]);
        }
        node[3]->SetSpan(pair<int,int>(0,3)); 
        {
            node[3]->AddEdge(edge[1]); 
            node[3]->AddEdge(edge[12]); 
            node[3]->AddEdge(edge[16]); 
            node[3]->AddEdge(edge[17]);
        }
        node[4]->SetSpan(pair<int,int>(1,2)); 
        {
            node[4]->AddEdge(edge[8]);
        }
        node[5]->SetSpan(pair<int,int>(1,3)); 
        {
            node[5]->AddEdge(edge[6]);
        }
        node[6]->SetSpan(pair<int,int>(1,4)); 
        {
            node[6]->AddEdge(edge[5]); 
            node[6]->AddEdge(edge[7]);
        }
        node[7]->SetSpan(pair<int,int>(2,3)); 
        {
            node[7]->AddEdge(edge[4]);
        }
        node[8]->SetSpan(pair<int,int>(2,4)); 
        {
            node[8]->AddEdge(edge[3]);
        }
        node[9]->SetSpan(pair<int,int>(3,4)); 
        {
            node[9]->AddEdge(edge[2]);
        }

        HyperGraph* expected_graph = new HyperGraph;
        BOOST_FOREACH(HyperEdge* ed, edge) {
            expected_graph->AddEdge(ed);   
        }

        BOOST_FOREACH(HyperNode* nd, node) {
            expected_graph->AddNode(nd);
        }

        BOOST_FOREACH(WordId w_id, c) {
            expected_graph->AddWord(w_id);
        }

        bool ret = actual_graph->CheckEqual(*expected_graph);
        return ret;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestBuildRules(lookup_fsm)" << endl; if(TestBuildRules(*lookup_fsm)) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestLookupTableFSM Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }
private:
    boost::scoped_ptr<LookupTableFSM> lookup_fsm;
    boost::scoped_ptr<LookupTableFSM> lookup_fsm_general;

};
}

#endif
