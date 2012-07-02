#ifndef TEST_LOOKUP_TABLE_H__
#define TEST_LOOKUP_TABLE_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <travatar/tree-io.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/translation-rule.h>
#include <boost/shared_ptr.hpp>

using namespace boost;

namespace travatar {

class TestLookupTable : public TestBase {

public:

    TestLookupTable() {
        // Use the example from Galley et al.
        string src1_tree = "(S (NP (PRP he)) (VP (AUX does) (RB not) (VB go)))";
        string trg1_str  = "il ne va pas";
        string align1_str = "0-0 1-1 1-3 2-1 2-3 3-2";
        istringstream iss1(src1_tree);
        src1_graph.reset(tree_io.ReadTree(iss1));
        trg1_sent = Dict::ParseWords(trg1_str);
        align1 = Alignment::FromString(align1_str);
        // Load the rules
        ostringstream rule_oss;
        rule_oss << "ROOT ( x0:S ) ||| x0 ||| 0.05 2.718" << endl;
        rule_oss << "S ( x0:NP x1:VP ) ||| x0 x1 ||| 0.1 2.718" << endl;
        rule_oss << "S ( x0:NP x1:VP ) ||| x1 x0 ||| 0.2 2.718" << endl;
        rule_oss << "S ( NP ( PRP ( \"he\" ) ) x0:VP ) ||| \"il\" x0 ||| 0.3 2.718" << endl;
        rule_oss << "NP ( x0:PRP ) ||| x0 ||| 0.4 2.718" << endl;
        rule_oss << "PRP ( \"he\" ) ||| \"il\" ||| 0.5 2.718" << endl;
        rule_oss << "VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB ) ||| \"ne\" x0 \"pas\" ||| 0.6 2.718" << endl;
        rule_oss << "VB ( \"go\" ) ||| \"va\" ||| 0.7 2.718" << endl;
        istringstream rule_iss(rule_oss.str());
        lookup_hash.reset(LookupTableHash::ReadFromRuleTable(rule_iss));
    }

    ~TestLookupTable() { }

    int TestHashLookup() {
        vector<int> exp_match_cnt(11, 0), act_match_cnt(11, 0);
        exp_match_cnt[0] = 2;
        exp_match_cnt[1] = 1;
        exp_match_cnt[2] = 1;
        exp_match_cnt[4] = 1;
        exp_match_cnt[9] = 1;
        vector<shared_ptr<LookupState> > old_states;
        old_states.push_back(shared_ptr<LookupState>(lookup_hash->GetInitialState()));
        for(int i = 0; i < 11; i++)
            act_match_cnt[i] = lookup_hash->LookupSrc(*src1_graph->GetNode(i), old_states).size();
        return CheckVector(exp_match_cnt, act_match_cnt);
    }

    int TestHashRules() {
        vector<vector<shared_ptr<LookupState> > > act_lookups(11);
        vector<shared_ptr<LookupState> > old_states;
        old_states.push_back(shared_ptr<LookupState>(lookup_hash->GetInitialState()));
        for(int i = 0; i < 11; i++)
            act_lookups[i] = lookup_hash->LookupSrc(*src1_graph->GetNode(i), old_states);
        vector<TranslationRule*> exp_rules(7), act_rules(7);
        act_rules[0] = SafeReference(lookup_hash->FindRules(*act_lookups[0][0]))[0]; // First S(NP, VP)
        act_rules[1] = SafeReference(lookup_hash->FindRules(*act_lookups[0][0]))[1]; // Second S(NP, VP)
        act_rules[2] = SafeReference(lookup_hash->FindRules(*act_lookups[0][1]))[0]; // First S(NP(PRP("he")) VP)
        act_rules[3] = SafeReference(lookup_hash->FindRules(*act_lookups[1][0]))[0]; // NP
        act_rules[4] = SafeReference(lookup_hash->FindRules(*act_lookups[2][0]))[0]; // PRP
        act_rules[5] = SafeReference(lookup_hash->FindRules(*act_lookups[4][0]))[0]; // VP
        act_rules[6] = SafeReference(lookup_hash->FindRules(*act_lookups[9][0]))[0]; // VB
        // Expected rules
        exp_rules[0] = new TranslationRule("S ( x0:NP x1:VP )");
        exp_rules[0]->AddTrgWord(-1); exp_rules[0]->AddTrgWord(-2);
        exp_rules[0]->AddFeature(0.1); exp_rules[0]->AddFeature(2.718);
        exp_rules[1] = new TranslationRule("S ( x0:NP x1:VP )");
        exp_rules[1]->AddTrgWord(-2); exp_rules[1]->AddTrgWord(-1);
        exp_rules[1]->AddFeature(0.2); exp_rules[1]->AddFeature(2.718);
        exp_rules[2] = new TranslationRule("S ( NP ( PRP ( \"he\" ) ) x0:VP )");
        exp_rules[2]->AddTrgWord(Dict::WID("il")); exp_rules[2]->AddTrgWord(-1);
        exp_rules[2]->AddFeature(0.3); exp_rules[2]->AddFeature(2.718);
        exp_rules[3] = new TranslationRule("NP ( x0:PRP )");
        exp_rules[3]->AddTrgWord(-1);
        exp_rules[3]->AddFeature(0.4); exp_rules[3]->AddFeature(2.718);
        exp_rules[4] = new TranslationRule("PRP ( \"he\" )");
        exp_rules[4]->AddTrgWord(Dict::WID("il"));
        exp_rules[4]->AddFeature(0.5); exp_rules[4]->AddFeature(2.718);
        exp_rules[5] = new TranslationRule("VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB )");
        exp_rules[5]->AddTrgWord(Dict::WID("ne")); exp_rules[5]->AddTrgWord(-1); exp_rules[5]->AddTrgWord(Dict::WID("pas"));
        exp_rules[5]->AddFeature(0.6); exp_rules[5]->AddFeature(2.718);
        exp_rules[6] = new TranslationRule("VB ( \"go\" )");
        exp_rules[6]->AddTrgWord(Dict::WID("va"));
        exp_rules[6]->AddFeature(0.7); exp_rules[6]->AddFeature(2.718);
        return CheckPtrVector(exp_rules, act_rules);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestHashLookup()" << endl; if(TestHashLookup()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestHashRules()" << endl; if(TestHashRules()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestLookupTable Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    PennTreeIO tree_io;
    boost::scoped_ptr<LookupTableHash> lookup_hash;
    boost::scoped_ptr<HyperGraph> src1_graph;
    Sentence trg1_sent;
    Alignment align1;

};

}

#endif
