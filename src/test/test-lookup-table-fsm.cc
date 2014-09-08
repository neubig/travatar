#include "test-lookup-table-fsm.h"
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/translation-rule-hiero.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

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
    lookup_fsm->SetTrgFactors(1);
    lookup_fsm->SetRootSymbol(Dict::WID("X"));
    lookup_fsm->SetDefaultSymbol(Dict::WID("X"));
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
    lookup_fsm_split->SetTrgFactors(1);
    lookup_fsm_split->SetRootSymbol(Dict::WID("X"));
    lookup_fsm_split->SetDefaultSymbol(Dict::WID("X"));
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
    lookup_fsm_extra->SetTrgFactors(1);
    lookup_fsm_extra->SetRootSymbol(Dict::WID("X"));
    lookup_fsm_extra->SetDefaultSymbol(Dict::WID("X"));
    lookup_fsm_extra->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_extra));

    // Load the rules, c = complete
    ostringstream rule_oss_c, rule_glue;
    rule_oss_c << "\"I\" x0:X @ X ||| \"watashi\" \"wa\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 1
    rule_oss_c << "\"eat\" \"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 2
    rule_oss_c << "\"two\" x0:X @ X ||| \"futatsu\" \"no\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 3
    rule_oss_c << "x0:X \"eat\" x1:X @ X ||| x0:X \"wa\" x1:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 4
    rule_oss_c << "\"eat\" x0:X @ X ||| x0:X \"wo\" \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 5
    rule_oss_c << "\"I\" x0:X \"two\" \"hamburgers\" @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" \"hanbaga\" \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 6
    rule_oss_c << "\"I\" x0:X \"two\" x1:X @ X ||| \"watashi\" \"wa\" \"futatsu\" \"no\" x1:X \"wo\" x0:X @ X ||| Pegf=0.02 ppen=2.718" << endl; // 7
    rule_oss_c << "\"I\" @ X ||| \"watashi\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 8
    rule_oss_c << "\"eat\" @ X ||| \"taberu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 9
    rule_oss_c << "\"two\" @ X ||| \"futatsu\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 10
    rule_oss_c << "\"hamburgers\" @ X ||| \"hanbaga\" @ X ||| Pegf=0.02 ppen=2.718" << endl; // 11
    rule_glue << "x0:X x1:X @ X ||| x0:X x1:X @ X ||| glue=1" << endl; // 12
    istringstream rule_iss_c(rule_oss_c.str());
    istringstream rule_iss_glue(rule_glue.str());
    lookup_fsm_c.reset(new LookupTableFSM);
    lookup_fsm_c->SetTrgFactors(1);
    lookup_fsm_c->SetRootSymbol(Dict::WID("X"));
    lookup_fsm_c->SetDefaultSymbol(Dict::WID("X"));
    lookup_fsm_c->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_c));
    lookup_fsm_c->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_glue));

    //Load the rule mhd = multi head
    ostringstream rule_oss_mhd;
    rule_oss_mhd << "\"the\" x0:NN x1:VP @ S ||| x1:NP x0:NN @ S ||| para=1" << endl;
    rule_oss_mhd << "\"program\" @ NN ||| \"program\" @ NN ||| para=1" << endl;
    rule_oss_mhd << "\"made\" \"by\" x0:PRP @ VP ||| x0:PRP$ @ NP ||| para=1" << endl;
    rule_oss_mhd << "\"me\" @ PRP ||| \"my\" @ PRP$ ||| para=1" << endl;
    istringstream rule_iss_mhd(rule_oss_mhd.str());
    lookup_fsm_mhd.reset(new LookupTableFSM);
    lookup_fsm_mhd->SetTrgFactors(1);
    lookup_fsm_mhd->SetRootSymbol(Dict::WID("S"));
    lookup_fsm_mhd->SetDefaultSymbol(Dict::WID("X"));
    lookup_fsm_mhd->AddRuleFSM(RuleFSM::ReadFromRuleTable(rule_iss_mhd));
}

TestLookupTableFSM::~TestLookupTableFSM() { }

boost::shared_ptr<TranslationRuleHiero> TestLookupTableFSM::BuildRule(const string & src, const string & trg, const string & feat) {
	return boost::shared_ptr<TranslationRuleHiero>(new TranslationRuleHiero(
        Dict::ParseAnnotatedVector(trg),
        Dict::ParseSparseVector(feat),
        Dict::ParseAnnotatedWords(src)
    ));
}

HyperGraph * TestLookupTableFSM::CreateExpectedGraph() {
    vector<HyperNode*> node(10);
    vector<HyperEdge*> edge(15);
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
    edge[0]->SetHead(node[0]);
    {
        edge[0]->AddTail(node[1]);
        edge[0]->AddTail(node[8]);
        edge[0]->SetRule(rules[5].get(), rules[5]->GetFeatures());
    }
    edge[1]->SetHead(node[3]);
    {
        edge[1]->AddTail(node[1]);
        edge[1]->AddTail(node[7]);
        edge[1]->SetRule(rules[5].get(), rules[5]->GetFeatures());
    }
    edge[2]->SetHead(node[9]);
    {
        edge[2]->SetRule(rules[10].get(), rules[10]->GetFeatures());
    }
    edge[3]->SetHead(node[8]);
    {
        edge[3]->AddTail(node[9]);
        edge[3]->SetRule(rules[8].get(), rules[8]->GetFeatures());
    }
    edge[4]->SetHead(node[7]);
    {
        edge[4]->SetRule(rules[9].get(), rules[9]->GetFeatures());
    }
    edge[5]->SetHead(node[6]);
    {
        edge[5]->AddTail(node[8]);
        edge[5]->SetRule(rules[6].get(), rules[6]->GetFeatures());
    }
    edge[6]->SetHead(node[5]);
    {
        edge[6]->AddTail(node[7]);
        edge[6]->SetRule(rules[6].get(), rules[6]->GetFeatures());
    }
    edge[7]->SetHead(node[6]);
    {
        edge[7]->AddTail(node[9]);
        edge[7]->SetRule(rules[4].get(), rules[4]->GetFeatures());
    }
    edge[8]->SetHead(node[4]);
    {
        edge[8]->SetRule(rules[7].get(), rules[7]->GetFeatures());
    }
    edge[9]->SetHead(node[0]);
    {
        edge[9]->AddTail(node[4]);
        edge[9]->AddTail(node[9]);
        edge[9]->SetRule(rules[3].get(), rules[3]->GetFeatures());
    }
    edge[10]->SetHead(node[0]);
    {
        edge[10]->AddTail(node[4]);
        edge[10]->SetRule(rules[1].get(), rules[1]->GetFeatures());
    }
    edge[11]->SetHead(node[0]);
    {
        edge[11]->AddTail(node[6]);
        edge[11]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    }
    edge[12]->SetHead(node[3]);
    {
        edge[12]->AddTail(node[5]);
        edge[12]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    }
    edge[13]->SetHead(node[2]);
    {
        edge[13]->AddTail(node[4]);
        edge[13]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    }
    edge[14]->SetHead(node[1]);
    {
        edge[14]->SetRule(rules[2].get(), rules[2]->GetFeatures());
    }
   
    node[0]->SetSpan(pair<int,int>(0,4));
    {
        node[0]->AddEdge(edge[0]);
        node[0]->AddEdge(edge[9]);
        node[0]->AddEdge(edge[10]);
        node[0]->AddEdge(edge[11]);
    }
    node[1]->SetSpan(pair<int,int>(0,1));
    {
        node[1]->AddEdge(edge[14]);
    }
    node[2]->SetSpan(pair<int,int>(0,2));
    {
        node[2]->AddEdge(edge[13]);
    }
    node[3]->SetSpan(pair<int,int>(0,3));
    {
        node[3]->AddEdge(edge[1]);
        node[3]->AddEdge(edge[12]);
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

    string inp = "I eat two hamburgers";
    Sentence c = Dict::ParseWords(inp);
    BOOST_FOREACH(WordId w_id, c) {
        expected_graph->AddWord(w_id);
    }
    return expected_graph;
}

HyperGraph * TestLookupTableFSM::CreateUnkExpectedGraph(bool del_unk) {
    vector<HyperNode*> node(10);
    vector<HyperEdge*> edge(21);
    vector<boost::shared_ptr<TranslationRuleHiero> > rules(13);

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
    rules[11] = boost::shared_ptr<TranslationRuleHiero>(LookupTableFSM::GetUnknownRule(del_unk ? Dict::WID("") : Dict::WID("three"), HieroHeadLabels(vector<WordId>(2,Dict::WID("X")))));
    rules[12] = BuildRule("x0:X x1:X @ X","x0:X x1:X @ X","glue=1");
    // TranslationRuleHiero* glue_rule = lookup.GetGlueRule();

    // Draw it. You will have an idea after you see the drawing.
    edge[0]->SetHead(node[7]);
    {
        edge[0]->SetRule(rules[11].get(), rules[11]->GetFeatures());
    }
    edge[1]->SetHead(node[9]);
    {
        edge[1]->SetRule(rules[10].get(), rules[10]->GetFeatures());
    }
    edge[2]->SetHead(node[8]);
    {
        edge[2]->AddTail(node[7]);
        edge[2]->AddTail(node[9]);
        edge[2]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[3]->SetHead(node[4]);
    {
        edge[3]->SetRule(rules[7].get(), rules[7]->GetFeatures());
    }
    edge[4]->SetHead(node[5]);
    {
        edge[4]->AddTail(node[7]);
        edge[4]->SetRule(rules[6].get(), rules[6]->GetFeatures());
    }
    edge[5]->SetHead(node[6]);
    {
        edge[5]->AddTail(node[8]);
        edge[5]->SetRule(rules[6].get(), rules[6]->GetFeatures());
    }
    edge[6]->SetHead(node[5]);
    {
        edge[6]->AddTail(node[4]);
        edge[6]->AddTail(node[7]);
        edge[6]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[7]->SetHead(node[6]);
    {
        edge[7]->AddTail(node[4]);
        edge[7]->AddTail(node[8]);
        edge[7]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[8]->SetHead(node[6]);
    {
        edge[8]->AddTail(node[5]);
        edge[8]->AddTail(node[9]);
        edge[8]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[9]->SetHead(node[1]);
    {
        edge[9]->SetRule(rules[2].get(), rules[2]->GetFeatures());
    }
    edge[10]->SetHead(node[2]);
    {
        edge[10]->AddTail(node[4]);
        edge[10]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    }
    edge[11]->SetHead(node[3]);
    {
        edge[11]->AddTail(node[5]);
        edge[11]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    }
    edge[12]->SetHead(node[0]);
    {
        edge[12]->AddTail(node[6]);
        edge[12]->SetRule(rules[0].get(), rules[0]->GetFeatures());
    }
    edge[13]->SetHead(node[3]);
    {
        edge[13]->AddTail(node[1]);
        edge[13]->AddTail(node[7]);
        edge[13]->SetRule(rules[5].get(), rules[5]->GetFeatures());
    }
    edge[14]->SetHead(node[0]);
    {
        edge[14]->AddTail(node[1]);
        edge[14]->AddTail(node[8]);
        edge[14]->SetRule(rules[5].get(), rules[5]->GetFeatures());
    }
    edge[15]->SetHead(node[2]);
    {
        edge[15]->AddTail(node[1]);
        edge[15]->AddTail(node[4]);
        edge[15]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[16]->SetHead(node[3]);
    {
        edge[16]->AddTail(node[1]);
        edge[16]->AddTail(node[5]);
        edge[16]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[17]->SetHead(node[0]);
    {
        edge[17]->AddTail(node[1]);
        edge[17]->AddTail(node[6]);
        edge[17]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[18]->SetHead(node[3]);
    {
        edge[18]->AddTail(node[2]);
        edge[18]->AddTail(node[7]);
        edge[18]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[19]->SetHead(node[0]);
    {
        edge[19]->AddTail(node[2]);
        edge[19]->AddTail(node[8]);
        edge[19]->SetRule(rules[12].get(), rules[12]->GetFeatures());
    }
    edge[20]->SetHead(node[0]);
    {
        edge[20]->AddTail(node[3]);
        edge[20]->AddTail(node[9]);
        edge[20]->SetRule(rules[12].get(), rules[12]->GetFeatures());
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
    return expected_graph;
}

HyperGraph * TestLookupTableFSM::CreateMultiHeadExpectedGraph() {
    vector<HyperNode*> node(7);
    vector<HyperEdge*> edge(7);
    vector<boost::shared_ptr<TranslationRuleHiero> > rules(13);

    for (int i=0; i < (int)node.size(); ++i) {
        node[i] = new HyperNode;
        node[i]->SetSym(Dict::WID("X"));
    }
    for (int j=0; j < (int)edge.size(); ++j) edge[j] = new HyperEdge;
    edge[0]->SetHead(node[4]);
    edge[1]->SetHead(node[5]);
    edge[2]->SetHead(node[6]);
    edge[3]->SetHead(node[3]);
    edge[4]->SetHead(node[1]);
    {
        edge[4]->AddTail(node[1]);
    }
    edge[5]->SetHead(node[2]);
    {
        edge[5]->AddTail(node[8]);
    }
    edge[6]->SetHead(node[0]);
    {
        edge[6]->AddTail(node[1]);
        edge[6]->AddTail(node[2]);
    }
    node[0]->SetSpan(pair<int,int>(0,5));
    {
        node[0]->SetSym(Dict::WID("S"));
        node[0]->AddEdge(edge[6]);
    }
    node[1]->SetSpan(pair<int,int>(2,5));
    {
        node[1]->SetSym(Dict::WID("VP"));
        node[1]->AddEdge(edge[4]);
    }
    node[2]->SetSpan(pair<int,int>(1,2));
    {
        node[2]->SetSym(Dict::WID("NN"));
        node[2]->AddEdge(edge[5]);
    }
    node[3]->SetSpan(pair<int,int>(4,5));
    {
        node[3]->SetSym(Dict::WID("PRP"));
        node[3]->AddEdge(edge[1]);
    }
    node[4]->SetSpan(pair<int,int>(0,1));
    {
        node[4]->AddEdge(edge[0]);
    }
    node[5]->SetSpan(pair<int,int>(2,3));
    {
        node[5]->AddEdge(edge[1]);
    }
    node[6]->SetSpan(pair<int,int>(3,4));
    {
        node[6]->AddEdge(edge[2]);
    }
    
    HyperGraph* expected_graph = new HyperGraph;
    BOOST_FOREACH(HyperEdge* ed, edge) {
        expected_graph->AddEdge(ed);
    }

    BOOST_FOREACH(HyperNode* nd, node) {
        expected_graph->AddNode(nd);
    }
    string inp = "the program made by me";
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

bool TestLookupTableFSM::TestUnkRules(LookupTableFSM & lookup, bool delete_unk) {
    string inp = "I eat three hamburgers";
    boost::shared_ptr<HyperGraph> input_graph(new HyperGraph);
    BOOST_FOREACH(WordId word, Dict::ParseWords(inp))
        input_graph->AddWord(word);
    
    bool prev_del_unk = lookup.GetDeleteUnknown();
    lookup.SetDeleteUnknown(delete_unk);
    HyperGraph* actual_graph = lookup.TransformGraph(*input_graph);
    HyperGraph* expected_graph = CreateUnkExpectedGraph(delete_unk);
    bool check_unk_correct = expected_graph->GetEdge(0)->GetTrgData() == actual_graph->GetEdge(0)->GetTrgData();
    if (!check_unk_correct) cerr << "Error in unknown rule, expect an empty translation: " << *actual_graph->GetEdge(0) << endl;
    bool ret = check_unk_correct && expected_graph->CheckMaybeEqual(*actual_graph);
    lookup.SetDeleteUnknown(prev_del_unk);
    delete actual_graph;
    delete expected_graph;
    return ret;
}

bool TestLookupTableFSM::TestMultiHead(LookupTableFSM & lookup) {
    string inp = "the program made by me";
    boost::shared_ptr<HyperGraph> input_graph(new HyperGraph);
    BOOST_FOREACH(WordId word, Dict::ParseWords(inp))
        input_graph->AddWord(word);
    
    HyperGraph* actual_graph = lookup.TransformGraph(*input_graph);
    HyperGraph* expected_graph = CreateMultiHeadExpectedGraph();
    bool ret = expected_graph->CheckMaybeEqual(*actual_graph);
    delete actual_graph;
    delete expected_graph;
    return ret;
}

bool TestLookupTableFSM::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestBuildRules(lookup_fsm)" << endl; if(TestBuildRules(*lookup_fsm)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestUnkRules(lookup_fsm)" << endl; if(TestUnkRules(*lookup_fsm_c,false)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestUnkRules(lookup_fsm,delete_unk)" << endl; if(TestUnkRules(*lookup_fsm_c,true)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBuildRules(lookup_fsm_split)" << endl; if(TestBuildRules(*lookup_fsm_split)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBuildRules(lookup_fsm_extra)" << endl; if(TestBuildRules(*lookup_fsm_extra)) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestMultiHead(lookup_fsm_mhd)" << endl; if (TestMultiHead(*lookup_fsm_mhd)) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestLookupTableFSM Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
} 
} // namespace travatar

