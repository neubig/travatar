#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule-hiero.h>
#include <travatar/lookup-table-cfglm.h>
#include <travatar/global-debug.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>
#include <lm/left.hh>
#include <string>

using namespace std;
using namespace boost;
using namespace travatar;

struct TestLookupTableCFGLM {
public:

    TestLookupTableCFGLM() { 
    
        // // Load the rules, c = complete
        // ostringstream rule_oss_c, rule_glue;
        // rule_oss_c << "\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
        // rule_oss_c << "\"eat\" \"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 2
        // rule_oss_c << "\"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 3
        // rule_oss_c << "x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 4
        // rule_oss_c << "\"eat\" x0:X @ X ||| x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 5
        // rule_oss_c << "\"I\" x0:X \"two\" \"hamburgers\" @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 6
        // rule_oss_c << "\"I\" x0:X \"two\" x1:X @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1:X \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 7
        // rule_oss_c << "\"I\" @ X ||| \"watashi\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 8
        // rule_oss_c << "\"eat\" @ X ||| \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 9
        // rule_oss_c << "\"two\" @ X ||| \"futatsu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 10
        // rule_oss_c << "\"hamburgers\" @ X ||| \"hanbaga\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 11
        // rule_glue << "x0:X x1:X @ X ||| x0:X x1:X @ X ||| glue=1" << endl; // 12
        // istringstream rule_iss_c(rule_oss_c.str());
        // istringstream rule_iss_glue(rule_glue.str());
        // lookup_cfglm_c.reset(new LookupTableCFGLM);
        // lookup_cfglm_c->SetTrgFactors(1);
        // lookup_cfglm_c->SetRootSymbol(Dict::WID("X"));
        // lookup_cfglm_c->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_c));
        // lookup_cfglm_c->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_glue));
    
    }
    
    ~TestLookupTableCFGLM() { }

    TranslationRuleHiero* BuildRule(const string & src, const string & trg, const string & feat) {
    	return new TranslationRuleHiero(
            Dict::ParseAnnotatedVector(trg),
            Dict::ParseSparseVector(feat),
            Dict::ParseAnnotatedWords(src)
        );
    }
    
    HyperGraph * CreateExpectedGraph(bool extra) {
        vector<HyperNode*> node(11 + (extra ? 1:0));
        vector<HyperEdge*> edge(14 + (extra ? 2:0));
        vector<TranslationRuleHiero*> rules(14);
    
        for (int i=0; i < (int)node.size(); ++i) node[i] = new HyperNode;
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
        rules[11] = new TranslationRuleHiero;

        rules[12] = BuildRule("\"two\" x0:DIFF @ X" ,"\"futatsu\" x0:DIFF @ X", "Pegf=0.02 ppen=2.718");
        rules[13] = BuildRule("\"hamburger\" @ DIFF","\"hamburger\" @ DIFF", "unk=1");
    
        // [3,4]: hamburgers
        edge[0]->SetHead(node[1]);
        edge[0]->SetRule(rules[10]);
        // [2,3]: two 
        edge[1]->SetHead(node[2]);
        edge[1]->SetRule(rules[9]);
        // [2,4]: two X0[3,4]
        edge[2]->SetHead(node[3]);
        edge[2]->AddTail(node[1]);
        edge[2]->SetRule(rules[8]);
        // [1,2]: eat
        edge[3]->SetHead(node[4]);
        edge[3]->SetRule(rules[7]);
        // [1,3]: eat X0[2,3]
        edge[4]->SetHead(node[5]);
        edge[4]->AddTail(node[2]);
        edge[4]->SetRule(rules[6]);
        // [1,4]: eat X0[2,4]
        edge[5]->SetHead(node[6]);
        edge[5]->AddTail(node[3]);
        edge[5]->SetRule(rules[6]);
        // [1,4]: eat two X0[3,4]
        edge[6]->SetHead(node[6]);
        edge[6]->AddTail(node[1]);
        edge[6]->SetRule(rules[4]);
        // [0,1]: I
        edge[7]->SetHead(node[7]);
        edge[7]->SetRule(rules[2]);
        // [0,2]: I X0[1,2]
        edge[8]->SetHead(node[8]);
        edge[8]->AddTail(node[4]);
        edge[8]->SetRule(rules[0]);
        // [0,3]: I X0[1,3]
        edge[9]->SetHead(node[9]);
        edge[9]->AddTail(node[5]);
        edge[9]->SetRule(rules[0]);
        // [0,4]: I X0[1,4]
        edge[10]->SetHead(node[10]);
        edge[10]->AddTail(node[6]);
        edge[10]->SetRule(rules[0]);
        // [0,4]: I X0[1,2] two X1[3,4]
        edge[11]->SetHead(node[10]);
        edge[11]->AddTail(node[4]);
        edge[11]->AddTail(node[1]);
        edge[11]->SetRule(rules[3]);
        // [0,4]: I X0[1,2] two hamburgers
        edge[12]->SetHead(node[10]);
        edge[12]->AddTail(node[4]);
        edge[12]->SetRule(rules[1]);
        // [0,4]: X0[0,4]
        edge[13]->SetHead(node[0]);
        edge[13]->AddTail(node[10]);
        edge[13]->SetTrgData(CfgDataVector(GlobalVars::trg_factors, CfgData(Sentence(1, -1))));
        // if (extra) {
        //     edge[10]->SetHead(node[6]);
        //     edge[10]->SetRule(rules[12]);
        //     edge[11]->SetHead(node[4]);
        //     edge[11]->AddTail(node[6]);
        //     edge[11]->SetRule(rules[11]);
        // }
       
        // root:
        node[0]->SetSpan(pair<int,int>(0,4));
        node[0]->AddEdge(edge[13]);
        node[0]->SetViterbiScore(0);
        // [3,4]:
        node[1]->SetSpan(pair<int,int>(3,4));
        node[1]->AddEdge(edge[0]);
        node[1]->SetViterbiScore(0);
        // [2,3]:
        node[2]->SetSpan(pair<int,int>(2,3));
        node[2]->AddEdge(edge[1]);
        node[2]->SetViterbiScore(0);
        // [2,4]:
        node[3]->SetSpan(pair<int,int>(2,4));
        node[3]->AddEdge(edge[2]);
        node[3]->SetViterbiScore(0);
        // [1,2]:
        node[4]->SetSpan(pair<int,int>(1,2));
        node[4]->AddEdge(edge[3]);
        node[4]->SetViterbiScore(0);
        // [1,3]:
        node[5]->SetSpan(pair<int,int>(1,3));
        node[5]->AddEdge(edge[4]);
        node[5]->SetViterbiScore(0);
        // [1,4]:
        node[6]->SetSpan(pair<int,int>(1,4));
        node[6]->AddEdge(edge[5]);
        node[6]->AddEdge(edge[6]);
        node[6]->SetViterbiScore(0);
        // [0,1]:
        node[7]->SetSpan(pair<int,int>(0,1));
        node[7]->AddEdge(edge[7]);
        // [0,2]:
        node[8]->SetSpan(pair<int,int>(0,2));
        node[8]->AddEdge(edge[8]);
        // [0,3]:
        node[9]->SetSpan(pair<int,int>(0,3));
        node[9]->AddEdge(edge[9]);
        // [0,4]:
        node[10]->SetSpan(pair<int,int>(0,4));
        node[10]->AddEdge(edge[10]);
        node[10]->AddEdge(edge[11]);
        node[10]->AddEdge(edge[12]);
        node[10]->SetViterbiScore(0);

        // if (extra) {
        //     node[6]->SetSpan(pair<int,int>(3,4));
        //     node[6]->AddEdge(edge[10]);
        // }
    
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
        BOOST_FOREACH(TranslationRule* rule, rules)
            delete rule;
        return expected_graph;
    }
    
    HyperGraph * CreateUnkExpectedGraph(bool del_unk) {
        vector<HyperNode*> node(10);
        vector<HyperEdge*> edge(21);
        vector<TranslationRuleHiero*> rules(13);
    
        for (int i=0; i < (int)node.size(); ++i) node[i] = new HyperNode;
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
        rules[11] = BuildRule("\"three\" @ X", del_unk ? ("@ X") : ("\"three\" @ X"),"unk=1");
        rules[12] = BuildRule("x0:X x1:X @ X","x0:X x1:X @ X","glue=1");
    
        // Draw it. You will have an idea after you see the drawing.
        edge[0]->SetHead(node[7]);
        {
            edge[0]->SetRule(rules[11]);
        }
        edge[1]->SetHead(node[9]);
        {
            edge[1]->SetRule(rules[10]);
        }
        edge[2]->SetHead(node[8]);
        {
            edge[2]->AddTail(node[7]);
            edge[2]->AddTail(node[9]);
            edge[2]->SetRule(rules[12]);
        }
        edge[3]->SetHead(node[4]);
        {
            edge[3]->SetRule(rules[7]);
        }
        edge[4]->SetHead(node[5]);
        {
            edge[4]->AddTail(node[7]);
            edge[4]->SetRule(rules[6]);
        }
        edge[5]->SetHead(node[6]);
        {
            edge[5]->AddTail(node[8]);
            edge[5]->SetRule(rules[6]);
        }
        edge[6]->SetHead(node[5]);
        {
            edge[6]->AddTail(node[4]);
            edge[6]->AddTail(node[7]);
            edge[6]->SetRule(rules[12]);
        }
        edge[7]->SetHead(node[6]);
        {
            edge[7]->AddTail(node[4]);
            edge[7]->AddTail(node[8]);
            edge[7]->SetRule(rules[12]);
        }
        edge[8]->SetHead(node[6]);
        {
            edge[8]->AddTail(node[5]);
            edge[8]->AddTail(node[9]);
            edge[8]->SetRule(rules[12]);
        }
        edge[9]->SetHead(node[1]);
        {
            edge[9]->SetRule(rules[2]);
        }
        edge[10]->SetHead(node[2]);
        {
            edge[10]->AddTail(node[4]);
            edge[10]->SetRule(rules[0]);
        }
        edge[11]->SetHead(node[3]);
        {
            edge[11]->AddTail(node[5]);
            edge[11]->SetRule(rules[0]);
        }
        edge[12]->SetHead(node[0]);
        {
            edge[12]->AddTail(node[6]);
            edge[12]->SetRule(rules[0]);
        }
        edge[13]->SetHead(node[3]);
        {
            edge[13]->AddTail(node[1]);
            edge[13]->AddTail(node[7]);
            edge[13]->SetRule(rules[5]);
        }
        edge[14]->SetHead(node[0]);
        {
            edge[14]->AddTail(node[1]);
            edge[14]->AddTail(node[8]);
            edge[14]->SetRule(rules[5]);
        }
        edge[15]->SetHead(node[2]);
        {
            edge[15]->AddTail(node[1]);
            edge[15]->AddTail(node[4]);
            edge[15]->SetRule(rules[12]);
        }
        edge[16]->SetHead(node[3]);
        {
            edge[16]->AddTail(node[1]);
            edge[16]->AddTail(node[5]);
            edge[16]->SetRule(rules[12]);
        }
        edge[17]->SetHead(node[0]);
        {
            edge[17]->AddTail(node[1]);
            edge[17]->AddTail(node[6]);
            edge[17]->SetRule(rules[12]);
        }
        edge[18]->SetHead(node[3]);
        {
            edge[18]->AddTail(node[2]);
            edge[18]->AddTail(node[7]);
            edge[18]->SetRule(rules[12]);
        }
        edge[19]->SetHead(node[0]);
        {
            edge[19]->AddTail(node[2]);
            edge[19]->AddTail(node[8]);
            edge[19]->SetRule(rules[12]);
        }
        edge[20]->SetHead(node[0]);
        {
            edge[20]->AddTail(node[3]);
            edge[20]->AddTail(node[9]);
            edge[20]->SetRule(rules[12]);
        }
        node[0]->SetSpan(pair<int,int>(0,4));
        {
            node[0]->AddEdge(edge[12]);
            node[0]->AddEdge(edge[14]);
            node[0]->AddEdge(edge[17]);
            node[0]->AddEdge(edge[20]);
        }
        node[1]->SetSpan(pair<int,int>(0,1));
        {
            node[1]->AddEdge(edge[9]);
        }
        node[2]->SetSpan(pair<int,int>(0,2));
        {
            node[2]->AddEdge(edge[10]);
            node[2]->AddEdge(edge[15]);
        }
        node[3]->SetSpan(pair<int,int>(0,3));
        {
            node[3]->AddEdge(edge[11]);
            node[3]->AddEdge(edge[13]);
            node[3]->AddEdge(edge[16]);
            node[3]->AddEdge(edge[18]);
        }
        node[4]->SetSpan(pair<int,int>(1,2));
        {
            node[4]->AddEdge(edge[3]);
        }
        node[5]->SetSpan(pair<int,int>(1,3));
        {
            node[5]->AddEdge(edge[4]);
            node[5]->AddEdge(edge[6]);
        }
        node[6]->SetSpan(pair<int,int>(1,4));
        {
            node[6]->AddEdge(edge[5]);
            node[6]->AddEdge(edge[7]);
            node[6]->AddEdge(edge[8]);
        }
        node[7]->SetSpan(pair<int,int>(2,3));
        {
            node[7]->AddEdge(edge[0]);
        }
        node[8]->SetSpan(pair<int,int>(2,4));
        {
            node[8]->AddEdge(edge[2]);
        }
        node[9]->SetSpan(pair<int,int>(3,4));
        {
            node[9]->AddEdge(edge[1]);
        }
    
        HyperGraph* expected_graph = new HyperGraph;
        BOOST_FOREACH(HyperEdge* ed, edge) {
            expected_graph->AddEdge(ed);
        }
    
        BOOST_FOREACH(HyperNode* nd, node) {
            expected_graph->AddNode(nd);
        }
    
        string inp = "I eat three hamburgers";
        Sentence c = Dict::ParseWords(inp);
        BOOST_FOREACH(WordId w_id, c) {
            expected_graph->AddWord(w_id);
        }
        BOOST_FOREACH(TranslationRule* rule, rules)
            delete rule;
        return expected_graph;
    }
    
    
    bool BuildRules(LookupTableCFGLM & lookup, bool extra = false) {
        string inp = "I eat two hamburgers";
        Sentence c = Dict::ParseWords(inp);
    
        boost::shared_ptr<HyperGraph> input_graph(new HyperGraph);
        BOOST_FOREACH(WordId word, Dict::ParseWords(inp)) 
            input_graph->AddWord(word);
    
        HyperGraph* actual_graph = lookup.TransformGraph(*input_graph);
        HyperGraph* expected_graph = CreateExpectedGraph(extra);
    
        bool ret = expected_graph->CheckMaybeEqual(*actual_graph);
        delete actual_graph;
        delete expected_graph;
        return ret;
    }
    
    // bool UnkRules(LookupTableCFGLM & lookup, bool delete_unk) {
    //     string inp = "I eat three hamburgers";
    //     boost::shared_ptr<HyperGraph> input_graph(new HyperGraph);
    //     BOOST_FOREACH(WordId word, Dict::ParseWords(inp))
    //         input_graph->AddWord(word);
    // 
    //     bool prev_del_unk = lookup.GetDeleteUnknown();
    //     lookup.SetDeleteUnknown(delete_unk);
    //     HyperGraph* actual_graph = lookup.TransformGraph(*input_graph);
    //     HyperGraph* expected_graph = CreateUnkExpectedGraph(delete_unk);
    //     bool ret = expected_graph->CheckMaybeEqual(*actual_graph);
    //     lookup.SetDeleteUnknown(prev_del_unk);
    //     delete actual_graph;
    //     delete expected_graph;
    //     return ret;
    // }
    
    boost::scoped_ptr<LookupTableCFGLM> lookup_cfglm;
    boost::scoped_ptr<LookupTableCFGLM> lookup_cfglm_split;
    boost::scoped_ptr<LookupTableCFGLM> lookup_cfglm_extra;
    boost::scoped_ptr<LookupTableCFGLM> lookup_cfglm_c;
};



BOOST_FIXTURE_TEST_SUITE(lookup_table_cfglm, TestLookupTableCFGLM)

BOOST_AUTO_TEST_CASE(TestBuildRules) {

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
    lookup_cfglm.reset(new LookupTableCFGLM);
    lookup_cfglm->SetTrgFactors(1);
    lookup_cfglm->SetRootSymbol(Dict::WID("X"));
    lookup_cfglm->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss));

    BOOST_CHECK(BuildRules(*lookup_cfglm));
}

// BOOST_AUTO_TEST_CASE(TestUnkRules) {
//     BOOST_CHECK(UnkRules(*lookup_cfglm_c,false));
// }
// 
// BOOST_AUTO_TEST_CASE(TestUnkRulesDelete) {
//     BOOST_CHECK(UnkRules(*lookup_cfglm_c,true));
// }

BOOST_AUTO_TEST_CASE(TestBuildRulesSplit) {

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
    lookup_cfglm_split.reset(new LookupTableCFGLM);
    lookup_cfglm_split->SetTrgFactors(1);
    lookup_cfglm_split->SetRootSymbol(Dict::WID("X"));
    lookup_cfglm_split->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss1));
    lookup_cfglm_split->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss2));

    BOOST_CHECK(BuildRules(*lookup_cfglm_split));
}

BOOST_AUTO_TEST_CASE(TestBuildRulesExtra) {

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
    rule_oss_extra << "\"two\" x0:DIFF @ X ||| \"futatsu\" x0:DIFF @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
    istringstream rule_iss_extra(rule_oss_extra.str());
    lookup_cfglm_extra.reset(new LookupTableCFGLM);
    lookup_cfglm_extra->SetTrgFactors(1);
    lookup_cfglm_extra->SetRootSymbol(Dict::WID("X"));
    lookup_cfglm_extra->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_extra));

    BOOST_CHECK(BuildRules(*lookup_cfglm_extra));
}

BOOST_AUTO_TEST_CASE(TestMultiHead) {

    //Load the rule mhd = multi head
    ostringstream rule_oss;
    rule_oss << "\"the\" x0:NN x1:VP @ S ||| x1:NP x0:NN @ S ||| para=1" << endl;
    rule_oss << "\"program\" @ NN ||| \"program\" @ NN ||| para=1" << endl;
    rule_oss << "\"made\" \"by\" x0:PRP @ VP ||| x0:PRP$ @ NP ||| para=1" << endl;
    rule_oss << "\"me\" @ PRP ||| \"my\" @ PRP$ ||| para=1" << endl;
    istringstream rule_iss(rule_oss.str());
    boost::scoped_ptr<LookupTableCFGLM> lookup(new LookupTableCFGLM);
    lookup->SetTrgFactors(1);
    lookup->SetRootSymbol(Dict::WID("S"));
    lookup->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss));

    string inp = "the program made by me";
    boost::scoped_ptr<HyperGraph> input_graph(new HyperGraph);
    BOOST_FOREACH(WordId word, Dict::ParseWords(inp))
        input_graph->AddWord(word);
    
    HyperGraph* actual_graph = lookup->TransformGraph(*input_graph);

    vector<HyperNode*> node(5);
    vector<HyperEdge*> edge(5);
    
    for (int i=0; i < (int)node.size(); ++i) node[i] = new HyperNode;
    for (int j=0; j < (int)edge.size(); ++j) edge[j] = new HyperEdge;

    // [4,5]: me
    edge[0]->SetHead(node[1]);
    // [2,5]: made by X0[4,5]
    edge[1]->SetHead(node[2]);
    edge[1]->AddTail(node[1]);
    // [1,2]: program
    edge[2]->SetHead(node[3]);
    // [0,5]: the X0[1,2] X1[2,5]
    edge[3]->SetHead(node[4]);
    edge[3]->AddTail(node[3]);
    edge[3]->AddTail(node[2]);
    // root:
    edge[4]->SetHead(node[0]);
    edge[4]->AddTail(node[4]);

    // root
    node[0]->SetSpan(pair<int,int>(0,5));
    node[0]->AddEdge(edge[4]);
    node[0]->SetViterbiScore(0);
    // [4,5]
    node[1]->SetSpan(pair<int,int>(4,5));
    node[1]->SetSym(Dict::WID("PRP"));
    node[1]->AddEdge(edge[0]);
    node[1]->SetViterbiScore(0);
    // [2,5]
    node[2]->SetSpan(pair<int,int>(2,5));
    node[2]->SetSym(Dict::WID("VP"));
    node[2]->AddEdge(edge[1]);
    node[2]->SetViterbiScore(0);
    // [1,2]
    node[3]->SetSpan(pair<int,int>(1,2));
    node[3]->SetSym(Dict::WID("NN"));
    node[3]->AddEdge(edge[2]);
    node[3]->SetViterbiScore(0);
    // [0,5]
    node[4]->SetSpan(pair<int,int>(0,5));
    node[4]->SetSym(Dict::WID("S"));
    node[4]->AddEdge(edge[3]);
    node[4]->SetViterbiScore(0);
    
    HyperGraph* expected_graph = new HyperGraph;
    BOOST_FOREACH(HyperEdge* ed, edge) { expected_graph->AddEdge(ed); }
    BOOST_FOREACH(HyperNode* nd, node) { expected_graph->AddNode(nd); }
    Sentence c = Dict::ParseWords(inp);
    BOOST_FOREACH(WordId w_id, c) { expected_graph->AddWord(w_id); }

    BOOST_CHECK(expected_graph->CheckMaybeEqual(*actual_graph));
    delete actual_graph;
    delete expected_graph;

}

BOOST_AUTO_TEST_CASE(TestUnary) {

    ostringstream rule_oss;
    rule_oss << "\"hello\" @ A ||| \"hello\" @ A ||| para=1" << endl;
    rule_oss << "x0:A @ B ||| x0:A @ B ||| para=1" << endl;
    rule_oss << "x0:B @ S ||| x0:B @ S ||| para=1" << endl;
    istringstream rule_iss(rule_oss.str());
    boost::scoped_ptr<LookupTableCFGLM> lookup(new LookupTableCFGLM);
    lookup->SetTrgFactors(1);
    lookup->SetRootSymbol(Dict::WID("S"));
    lookup->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss));

    string inp = "hello";
    boost::scoped_ptr<HyperGraph> input_graph(new HyperGraph);
    BOOST_FOREACH(WordId word, Dict::ParseWords(inp))
        input_graph->AddWord(word);
    
    HyperGraph* actual_graph = lookup->TransformGraph(*input_graph);

    vector<HyperNode*> node(4);
    vector<HyperEdge*> edge(4);
    
    for (int i=0; i < (int)node.size(); ++i) node[i] = new HyperNode;
    for (int j=0; j < (int)edge.size(); ++j) edge[j] = new HyperEdge;

    // [0,1]: hello -> A
    edge[0]->SetHead(node[1]);
    // [0,1]: A -> B
    edge[1]->SetHead(node[2]);
    edge[1]->AddTail(node[1]);
    // [0,1]: B -> S
    edge[2]->SetHead(node[3]);
    edge[2]->AddTail(node[2]);
    // [0,1]: S -> root
    edge[3]->SetHead(node[0]);
    edge[3]->AddTail(node[3]);

    // root
    node[0]->SetSpan(pair<int,int>(0,1));
    node[0]->AddEdge(edge[3]);
    node[0]->SetViterbiScore(0);
    // [0,1]A
    node[1]->SetSpan(pair<int,int>(0,1));
    node[1]->SetSym(Dict::WID("A"));
    node[1]->AddEdge(edge[0]);
    node[1]->SetViterbiScore(0);
    // [0,1]B
    node[2]->SetSpan(pair<int,int>(0,1));
    node[2]->SetSym(Dict::WID("B"));
    node[2]->AddEdge(edge[1]);
    node[2]->SetViterbiScore(0);
    // [0,1]S
    node[3]->SetSpan(pair<int,int>(0,1));
    node[3]->SetSym(Dict::WID("S"));
    node[3]->AddEdge(edge[2]);
    node[3]->SetViterbiScore(0);
    
    HyperGraph* expected_graph = new HyperGraph;
    BOOST_FOREACH(HyperEdge* ed, edge) { expected_graph->AddEdge(ed); }
    BOOST_FOREACH(HyperNode* nd, node) { expected_graph->AddNode(nd); }

    Sentence c = Dict::ParseWords(inp);
    BOOST_FOREACH(WordId w_id, c) { expected_graph->AddWord(w_id); }

    BOOST_CHECK(expected_graph->CheckMaybeEqual(*actual_graph));
    delete actual_graph;
    delete expected_graph;

}

BOOST_AUTO_TEST_SUITE_END()


