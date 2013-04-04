#ifndef TEST_LOOKUP_TABLE_H__
#define TEST_LOOKUP_TABLE_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/lookup-table-marisa.h>
#include <travatar/translation-rule.h>
#include <boost/shared_ptr.hpp>

using namespace boost;

namespace travatar {

class TestLookupTable : public TestBase {

public:

    TestLookupTable() {
        // Use the example from Galley et al.
        string src1_tree = "{\"nodes\": [{\"sym\": \"S\", \"span\": [0, 4], \"id\": 0, \"edges\": [0]}, {\"sym\": \"NP\", \"span\": [0, 1], \"id\": 1, \"edges\": [1]}, {\"sym\": \"PRP\", \"span\": [0, 1], \"id\": 2, \"edges\": [2]}, {\"sym\": \"he\", \"span\": [0, 1], \"id\": 3}, {\"sym\": \"VP\", \"span\": [1, 4], \"id\": 4, \"edges\": [3]}, {\"sym\": \"AUX\", \"span\": [1, 2], \"id\": 5, \"edges\": [4]}, {\"sym\": \"does\", \"span\": [1, 2], \"id\": 6}, {\"sym\": \"RB\", \"span\": [2, 3], \"id\": 7, \"edges\": [5]}, {\"sym\": \"not\", \"span\": [2, 3], \"id\": 8}, {\"sym\": \"VB\", \"span\": [3, 4], \"id\": 9, \"edges\": [6]}, {\"sym\": \"go\", \"span\": [3, 4], \"id\": 10}], \"edges\": [{\"id\": 0, \"head\": 0, \"tails\": [1, 4]}, {\"id\": 1, \"head\": 1, \"tails\": [2]}, {\"id\": 2, \"head\": 2, \"tails\": [3]}, {\"id\": 3, \"head\": 4, \"tails\": [5, 7, 9]}, {\"id\": 4, \"head\": 5, \"tails\": [6], \"features\": {\"parse\": 1}}, {\"id\": 5, \"head\": 7, \"tails\": [8], \"features\": {\"parse\": 1}}, {\"id\": 6, \"head\": 9, \"tails\": [10]}], \"words\": [\"he\", \"does\", \"not\", \"go\"]}";
        string trg1_str  = "il ne va pas";
        string align1_str = "0-0 1-1 1-3 2-1 2-3 3-2";
        istringstream iss1(src1_tree);
        src1_graph.reset(tree_io.ReadTree(iss1));
        trg1_sent = Dict::ParseWords(trg1_str);
        align1 = Alignment::FromString(align1_str);
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "ROOT ( x0:S ) ||| x0 ||| Pegf=0.05 ppen=2.718" << endl;
        rule_oss << "S ( x0:NP x1:VP ) ||| x0 x1 ||| Pegf=0.1 ppen=2.718" << endl;
        rule_oss << "S ( x0:NP x1:VP ) ||| x1 x0 ||| Pegf=0.2 ppen=2.718" << endl;
        rule_oss << "S ( NP ( PRP ( \"he\" ) ) x0:VP ) ||| \"il\" x0 ||| Pegf=0.3 ppen=2.718" << endl;
        rule_oss << "NP ( x0:PRP ) ||| x0 ||| Pegf=0.4 ppen=2.718" << endl;
        rule_oss << "PRP ( \"he\" ) ||| \"il\" ||| Pegf=0.5 ppen=2.718" << endl;
        rule_oss << "VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB ) ||| \"ne\" x0 \"pas\" ||| Pegf=0.6 ppen=2.718" << endl;
        rule_oss << "VB ( \"go\" ) ||| \"va\" ||| Pegf=0.7 ppen=2.718" << endl;
        istringstream rule_iss_hash(rule_oss.str());
        lookup_hash.reset(LookupTableHash::ReadFromRuleTable(rule_iss_hash));
        istringstream rule_iss_marisa(rule_oss.str());
        lookup_marisa.reset(LookupTableMarisa::ReadFromRuleTable(rule_iss_marisa));
    }

    ~TestLookupTable() { }

    int TestLookup(LookupTable & lookup) {
        vector<int> exp_match_cnt(11, 0), act_match_cnt(11, 0);
        exp_match_cnt[0] = 2;
        exp_match_cnt[1] = 1;
        exp_match_cnt[2] = 1;
        exp_match_cnt[4] = 1;
        exp_match_cnt[9] = 1;
        vector<shared_ptr<LookupState> > old_states;
        old_states.push_back(shared_ptr<LookupState>(lookup.GetInitialState()));
        for(int i = 0; i < 11; i++)
            act_match_cnt[i] = lookup.LookupSrc(*src1_graph->GetNode(i), old_states).size();
        return CheckVector(exp_match_cnt, act_match_cnt);
    }

    int TestLookupRules(LookupTable & lookup) {
        vector<vector<shared_ptr<LookupState> > > act_lookups(11);
        vector<shared_ptr<LookupState> > old_states;
        old_states.push_back(shared_ptr<LookupState>(lookup.GetInitialState()));
        for(int i = 0; i < 11; i++)
            act_lookups[i] = lookup.LookupSrc(*src1_graph->GetNode(i), old_states);
        vector<TranslationRule*> exp_rules(7), act_rules(7);
        act_rules[0] = SafeReference(lookup.FindRules(*act_lookups[0][0]))[0]; // First S(NP, VP)
        act_rules[1] = SafeReference(lookup.FindRules(*act_lookups[0][0]))[1]; // Second S(NP, VP)
        act_rules[2] = SafeReference(lookup.FindRules(*act_lookups[0][1]))[0]; // First S(NP(PRP("he")) VP)
        act_rules[3] = SafeReference(lookup.FindRules(*act_lookups[1][0]))[0]; // NP
        act_rules[4] = SafeReference(lookup.FindRules(*act_lookups[2][0]))[0]; // PRP
        act_rules[5] = SafeReference(lookup.FindRules(*act_lookups[4][0]))[0]; // VP
        act_rules[6] = SafeReference(lookup.FindRules(*act_lookups[9][0]))[0]; // VB
        // Expected rules
        exp_rules[0] = new TranslationRule("S ( x0:NP x1:VP )");
        exp_rules[0]->AddTrgWord(-1); exp_rules[0]->AddTrgWord(-2);
        exp_rules[0]->AddFeature("Pegf", 0.1); exp_rules[0]->AddFeature("ppen", 2.718);
        exp_rules[1] = new TranslationRule("S ( x0:NP x1:VP )");
        exp_rules[1]->AddTrgWord(-2); exp_rules[1]->AddTrgWord(-1);
        exp_rules[1]->AddFeature("Pegf", 0.2); exp_rules[1]->AddFeature("ppen", 2.718);
        exp_rules[2] = new TranslationRule("S ( NP ( PRP ( \"he\" ) ) x0:VP )");
        exp_rules[2]->AddTrgWord(Dict::WID("il")); exp_rules[2]->AddTrgWord(-1);
        exp_rules[2]->AddFeature("Pegf", 0.3); exp_rules[2]->AddFeature("ppen", 2.718);
        exp_rules[3] = new TranslationRule("NP ( x0:PRP )");
        exp_rules[3]->AddTrgWord(-1);
        exp_rules[3]->AddFeature("Pegf", 0.4); exp_rules[3]->AddFeature("ppen", 2.718);
        exp_rules[4] = new TranslationRule("PRP ( \"he\" )");
        exp_rules[4]->AddTrgWord(Dict::WID("il"));
        exp_rules[4]->AddFeature("Pegf", 0.5); exp_rules[4]->AddFeature("ppen", 2.718);
        exp_rules[5] = new TranslationRule("VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB )");
        exp_rules[5]->AddTrgWord(Dict::WID("ne")); exp_rules[5]->AddTrgWord(-1); exp_rules[5]->AddTrgWord(Dict::WID("pas"));
        exp_rules[5]->AddFeature("Pegf", 0.6); exp_rules[5]->AddFeature("ppen", 2.718);
        exp_rules[6] = new TranslationRule("VB ( \"go\" )");
        exp_rules[6]->AddTrgWord(Dict::WID("va"));
        exp_rules[6]->AddFeature("Pegf", 0.7); exp_rules[6]->AddFeature("ppen", 2.718);
        int ret =  CheckPtrVector(exp_rules, act_rules);
        BOOST_FOREACH(TranslationRule * rule, exp_rules)
            delete rule;
        return ret;
    }

    int TestBuildRuleGraph() {
        // Make the rule graph
        shared_ptr<HyperGraph> act_rule_graph(lookup_hash->TransformGraph(*src1_graph));
        vector<vector<shared_ptr<LookupState> > > act_lookups(11);
        vector<shared_ptr<LookupState> > old_states;
        old_states.push_back(shared_ptr<LookupState>(lookup_hash->GetInitialState()));
        for(int i = 0; i < 11; i++)
            act_lookups[i] = lookup_hash->LookupSrc(*src1_graph->GetNode(i), old_states);
        // Create the rule graph
        // string src1_tree = "(S0 (NP1 (PRP2 he3)) (VP4 (AUX5 does6) (RB7 not8) (VB9 go10)))";
        HyperGraph exp_rule_graph;
        exp_rule_graph.SetWords(src1_graph->GetWords());
        // Create the nodes to represent the non-terminals
        HyperNode * s0_node = new HyperNode; s0_node->SetSym(Dict::WID("S")); s0_node->SetSpan(src1_graph->GetNode(0)->GetSpan()); exp_rule_graph.AddNode(s0_node);
        HyperNode * np1_node = new HyperNode; np1_node->SetSym(Dict::WID("NP")); np1_node->SetSpan(src1_graph->GetNode(1)->GetSpan()); exp_rule_graph.AddNode(np1_node);
        HyperNode * prp2_node = new HyperNode; prp2_node->SetSym(Dict::WID("PRP")); prp2_node->SetSpan(src1_graph->GetNode(2)->GetSpan()); exp_rule_graph.AddNode(prp2_node);
        HyperNode * vp4_node = new HyperNode; vp4_node->SetSym(Dict::WID("VP")); vp4_node->SetSpan(src1_graph->GetNode(4)->GetSpan()); exp_rule_graph.AddNode(vp4_node);
        HyperNode * aux5_node = new HyperNode; aux5_node->SetSym(Dict::WID("AUX")); aux5_node->SetSpan(src1_graph->GetNode(5)->GetSpan()); exp_rule_graph.AddNode(aux5_node);
        HyperNode * rb7_node = new HyperNode; rb7_node->SetSym(Dict::WID("RB")); rb7_node->SetSpan(src1_graph->GetNode(7)->GetSpan()); exp_rule_graph.AddNode(rb7_node);
        HyperNode * vb9_node = new HyperNode; vb9_node->SetSym(Dict::WID("VB")); vb9_node->SetSpan(src1_graph->GetNode(9)->GetSpan()); exp_rule_graph.AddNode(vb9_node);
        // Gather the rules
        vector<TranslationRule*> act_rules(7);
        act_rules[0] = SafeReference(lookup_hash->FindRules(*act_lookups[0][0]))[0]; // First S(NP, VP)
        act_rules[1] = SafeReference(lookup_hash->FindRules(*act_lookups[0][0]))[1]; // Second S(NP, VP)
        act_rules[2] = SafeReference(lookup_hash->FindRules(*act_lookups[0][1]))[0]; // First S(NP(PRP("he")) VP)
        act_rules[3] = SafeReference(lookup_hash->FindRules(*act_lookups[1][0]))[0]; // NP
        act_rules[4] = SafeReference(lookup_hash->FindRules(*act_lookups[2][0]))[0]; // PRP
        act_rules[5] = SafeReference(lookup_hash->FindRules(*act_lookups[4][0]))[0]; // VP
        act_rules[6] = SafeReference(lookup_hash->FindRules(*act_lookups[9][0]))[0]; // VB
        // Create the HyperEdges
        HyperEdge* s0_edge = new HyperEdge(s0_node); s0_edge->AddTail(np1_node); s0_edge->AddTail(vp4_node); s0_node->AddEdge(s0_edge); exp_rule_graph.AddEdge(s0_edge); s0_edge->SetRule(act_rules[0]);
        HyperEdge* s0_edge_rev = new HyperEdge(s0_node); s0_edge_rev->AddTail(np1_node); s0_edge_rev->AddTail(vp4_node); s0_node->AddEdge(s0_edge_rev); exp_rule_graph.AddEdge(s0_edge_rev); s0_edge_rev->SetRule(act_rules[1]);
        HyperEdge* s0_edge_big = new HyperEdge(s0_node); s0_edge_big->AddTail(vp4_node); s0_node->AddEdge(s0_edge_big); exp_rule_graph.AddEdge(s0_edge_big); s0_edge_big->SetRule(act_rules[2]);
        HyperEdge* np1_edge = new HyperEdge(np1_node); np1_edge->AddTail(prp2_node); np1_node->AddEdge(np1_edge); exp_rule_graph.AddEdge(np1_edge); np1_edge->SetRule(act_rules[3]);
        HyperEdge* prp2_edge = new HyperEdge(prp2_node); prp2_node->AddEdge(prp2_edge); exp_rule_graph.AddEdge(prp2_edge); prp2_edge->SetRule(act_rules[4]);
        HyperEdge* vp4_edge = new HyperEdge(vp4_node); vp4_edge->AddTail(vb9_node); vp4_node->AddEdge(vp4_edge); exp_rule_graph.AddEdge(vp4_edge); vp4_edge->SetRule(act_rules[5]);  vp4_edge->AddFeature(Dict::WID("parse"), 2.0);
        HyperEdge* aux5_edge = new HyperEdge(aux5_node); aux5_node->AddEdge(aux5_edge); exp_rule_graph.AddEdge(aux5_edge); aux5_edge->SetRule(lookup_hash->GetUnknownRule()); aux5_edge->AddFeature(Dict::WID("parse"), 1.0); // Unknown edge
        HyperEdge* rb7_edge = new HyperEdge(rb7_node); rb7_node->AddEdge(rb7_edge); exp_rule_graph.AddEdge(rb7_edge); rb7_edge->SetRule(lookup_hash->GetUnknownRule());  rb7_edge->AddFeature(Dict::WID("parse"), 1.0); // Unknown edge
        HyperEdge* vb9_edge = new HyperEdge(vb9_node); vb9_node->AddEdge(vb9_edge); exp_rule_graph.AddEdge(vb9_edge); vb9_edge->SetRule(act_rules[6]);
        return exp_rule_graph.CheckEqual(*act_rule_graph);
    }

    int TestBadInputHash() {
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "ROOT ( x0:S ) ||| x0" << endl;
        istringstream rule_iss_hash(rule_oss.str());
        try {
            shared_ptr<LookupTableHash> temp(LookupTableHash::ReadFromRuleTable(rule_iss_hash));
            return 0;
        } catch(std::runtime_error e) {
            return 1;
        }
    }

    int TestBadInputMarisa() {
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "ROOT ( x0:S ) ||| x0" << endl;
        istringstream rule_iss_marisa(rule_oss.str());
        try {
            shared_ptr<LookupTableMarisa> temp(LookupTableMarisa::ReadFromRuleTable(rule_iss_marisa));
            return 0;
        } catch(std::runtime_error e) {
            return 1;
        }
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestLookup(lookup_hash)" << endl; if(TestLookup(*lookup_hash)) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestLookup(lookup_marisa)" << endl; if(TestLookup(*lookup_marisa)) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestLookupRules(lookup_hash)" << endl; if(TestLookupRules(*lookup_hash)) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestLookupRules(lookup_marisa)" << endl; if(TestLookupRules(*lookup_marisa)) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestBadInputHash()" << endl; if(TestBadInputHash()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestBadInputMarisa()" << endl; if(TestBadInputMarisa()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestBuildRuleGraph()" << endl; if(TestBuildRuleGraph()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestLookupTable Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    JSONTreeIO tree_io;
    boost::scoped_ptr<LookupTableHash> lookup_hash;
    boost::scoped_ptr<LookupTableMarisa> lookup_marisa;
    boost::scoped_ptr<HyperGraph> src1_graph;
    Sentence trg1_sent;
    Alignment align1;

};

}

#endif
