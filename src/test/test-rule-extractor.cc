#include "test-rule-extractor.h"
#include <travatar/global-debug.h>
#include <travatar/check-equal.h>
#include <boost/shared_ptr.hpp>
#include <sstream>

using namespace std;

namespace travatar {

TestRuleExtractor::TestRuleExtractor() {
    // Use the example from Galley et al.
    std::string src1_tree = "(ROOT (S (NP (PRP he)) (VP (AUX does) (RB not) (VB go))))";
    std::string trg1_str  = "il ne va pas";
    std::string trg1_tree = "(ROOT (S (NP (PRP il)) (VP (RB ne) (VB va) (AUX pas))))";
    std::string align1_str = "0-0 1-1 1-3 2-1 2-3 3-2";
    istringstream iss1(src1_tree);
    src1_graph.reset(tree_io.ReadTree(iss1));
    istringstream iss1_trg(trg1_tree);
    trg1_graph.reset(tree_io.ReadTree(iss1_trg));
    trg1_sent = Dict::ParseWords(trg1_str);
    align1 = Alignment::FromString(align1_str);
    // also an example of a forest
    src2_graph.reset(new HyperGraph);
    HyperNode* node0 = new HyperNode(Dict::WID("ROOT"), -1,    make_pair(0,2)); src2_graph->AddNode(node0);
    HyperNode* node1 = new HyperNode(Dict::WID("VP"), -1,      make_pair(0,2)); src2_graph->AddNode(node1);
    HyperNode* node2 = new HyperNode(Dict::WID("NP"), -1,      make_pair(0,2)); src2_graph->AddNode(node2);
    HyperNode* node3 = new HyperNode(Dict::WID("JJ"), -1,      make_pair(0,1)); src2_graph->AddNode(node3);
    HyperNode* node4 = new HyperNode(Dict::WID("VP"), -1,      make_pair(0,1)); src2_graph->AddNode(node4);
    HyperNode* node5 = new HyperNode(Dict::WID("VPG"), -1,     make_pair(0,1)); src2_graph->AddNode(node5);
    HyperNode* node6 = new HyperNode(Dict::WID("running"), -1, make_pair(0,1)); src2_graph->AddNode(node6);
    HyperNode* node7 = new HyperNode(Dict::WID("NP"), -1,      make_pair(1,2)); src2_graph->AddNode(node7);
    HyperNode* node8 = new HyperNode(Dict::WID("NN"), -1,      make_pair(1,2)); src2_graph->AddNode(node8);
    HyperNode* node9 = new HyperNode(Dict::WID("water"), -1,   make_pair(1,2)); src2_graph->AddNode(node9);
    HyperEdge* edge0 = new HyperEdge(node0); edge0->AddTail(node1); node0->AddEdge(edge0); src2_graph->AddEdge(edge0);
    HyperEdge* edge1 = new HyperEdge(node0); edge1->AddTail(node2); node0->AddEdge(edge1); src2_graph->AddEdge(edge1);
    HyperEdge* edge2 = new HyperEdge(node1); edge2->AddTail(node4); edge2->AddTail(node7); node1->AddEdge(edge2); src2_graph->AddEdge(edge2);
    HyperEdge* edge3 = new HyperEdge(node2); edge3->AddTail(node3); edge3->AddTail(node8); node2->AddEdge(edge3); src2_graph->AddEdge(edge3);
    HyperEdge* edge4 = new HyperEdge(node3); edge4->AddTail(node6); node3->AddEdge(edge4); src2_graph->AddEdge(edge4);
    HyperEdge* edge5 = new HyperEdge(node4); edge5->AddTail(node5); node4->AddEdge(edge5); src2_graph->AddEdge(edge5);
    HyperEdge* edge6 = new HyperEdge(node5); edge6->AddTail(node6); node5->AddEdge(edge6); src2_graph->AddEdge(edge6);
    HyperEdge* edge7 = new HyperEdge(node7); edge7->AddTail(node8); node7->AddEdge(edge7); src2_graph->AddEdge(edge7);
    HyperEdge* edge8 = new HyperEdge(node8); edge8->AddTail(node9); node8->AddEdge(edge8); src2_graph->AddEdge(edge8);
    src2_graph->SetWords(Dict::ParseWords("running water"));
    std::string trg2_str  = "mizu no nagare"; // Handle this translation with caution :)
    std::string align2_str = "0-2 1-0";
    trg2_sent = Dict::ParseWords(trg2_str);
    align2 = Alignment::FromString(align2_str);
    // An example of a tree with source and target side null words
    std::string src3_tree = "(ROOT (VP (VBD ate) (NP (NN rice))))";
    std::string trg3_str  = "gohan o tabeta";
    std::string align3_str = "0-2 1-0";
    istringstream iss3(src3_tree);
    src3_graph.reset(tree_io.ReadTree(iss3));
    trg3_sent = Dict::ParseWords(trg3_str);
    align3 = Alignment::FromString(align3_str);
    // An example of a tree that caused problems
    std::string src4_tree =
"{\"nodes\": ["
    "{\"sym\": \"root\", \"span\": [0, 5], \"id\": 0, \"edges\": [0]},"
    "{\"sym\": \"np\", \"span\": [0, 5], \"id\": 1, \"edges\": [1]},"
    "{\"sym\": \"np\", \"span\": [0, 1], \"id\": 2, \"edges\": [2]},"
    "{\"sym\": \"prn\", \"span\": [1, 5], \"id\": 3, \"edges\": [5, 6]},"
    "{\"sym\": \"nnp\", \"span\": [0, 1], \"id\": 4, \"edges\": [3]},"
    "{\"sym\": \"landscape\", \"span\": [0, 1], \"id\": 5},"
    "{\"sym\": \"prn'\", \"span\": [1, 4], \"id\": 6, \"edges\": [4]},"
    "{\"sym\": \"-lrb-\", \"span\": [1, 2], \"id\": 7, \"edges\": [8]},"
    "{\"sym\": \"np\", \"span\": [2, 4], \"id\": 8, \"edges\": [9]},"
    "{\"sym\": \"prn'\", \"span\": [2, 5], \"id\": 9, \"edges\": [7]},"
    "{\"sym\": \"-rrb-\", \"span\": [4, 5], \"id\": 10, \"edges\": [12]},"
    "{\"sym\": \"-lrb-\", \"span\": [1, 2], \"id\": 11},"
    "{\"sym\": \"nnp\", \"span\": [2, 3], \"id\": 12, \"edges\": [10]},"
    "{\"sym\": \"nnp\", \"span\": [3, 4], \"id\": 13, \"edges\": [11]},"
    "{\"sym\": \"private\", \"span\": [2, 3], \"id\": 14},"
    "{\"sym\": \"collection\", \"span\": [3, 4], \"id\": 15},"
    "{\"sym\": \"-rrb-\", \"span\": [4, 5], \"id\": 16}"
"], \"edges\": ["
    "{\"id\": 0, \"head\": 0, \"tails\": [1]},"
    "{\"id\": 1, \"head\": 1, \"tails\": [2, 3]},"
    "{\"id\": 2, \"head\": 2, \"tails\": [4]},"
    "{\"id\": 3, \"head\": 4, \"tails\": [5]},"
    "{\"id\": 4, \"head\": 6, \"tails\": [7, 8]},"
    "{\"id\": 5, \"head\": 3, \"tails\": [7, 9]},"
    "{\"id\": 6, \"head\": 3, \"tails\": [6, 10]},"
    "{\"id\": 7, \"head\": 9, \"tails\": [8, 10]},"
    "{\"id\": 8, \"head\": 7, \"tails\": [11]},"
    "{\"id\": 9, \"head\": 8, \"tails\": [12, 13]},"
    "{\"id\": 10, \"head\": 12, \"tails\": [14]},"
    "{\"id\": 11, \"head\": 13, \"tails\": [15]},"
    "{\"id\": 12, \"head\": 10, \"tails\": [16]}"
"], \"words\": ["
    "\"landscape\","
    "\"-lrb-\","
    "\"private\","
    "\"collection\","
    "\"-rrb-\""
"]}";
    std::string trg4_str = "sansui zu ( kojin zo )";
    std::string align4_str = "0-0 0-1 1-2 2-3 2-4 3-3 4-5";
    istringstream iss4(src4_tree);
    src4_graph.reset(json_io.ReadTree(iss4));
    trg4_sent = Dict::ParseWords(trg4_str);
    align4 = Alignment::FromString(align4_str);
}

TestRuleExtractor::~TestRuleExtractor() { }

int TestRuleExtractor::TestTreeExtraction() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::scoped_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src1_graph, align1));
    // Create the actual rule graph
    HyperGraph frags_exp;
    frags_exp.SetWords(src1_graph->GetWords());
    // ---- Add nodes ----
    // Expected node numbers: "(ROOT0 (S1 (NP2 (PRP3 he)) (VP4 (AUX5 does) (RB6 not) (VB7 go))))";
    // Node rooted at root0
    HyperNode* root0_node = new HyperNode(Dict::WID("ROOT"), -1, make_pair(0,4));
    root0_node->SetTrgSpan(src1_graph->GetNode(0)->GetTrgSpan());
    frags_exp.AddNode(root0_node);
    // Node rooted at s1
    HyperNode* s1_node = new HyperNode(Dict::WID("S"), -1, make_pair(0,4));
    s1_node->SetTrgSpan(src1_graph->GetNode(1)->GetTrgSpan());
    frags_exp.AddNode(s1_node);
    // Node rooted at np2
    HyperNode* np2_node = new HyperNode(Dict::WID("NP"), -1, make_pair(0,1));
    np2_node->SetTrgSpan(src1_graph->GetNode(2)->GetTrgSpan());
    frags_exp.AddNode(np2_node);
    // Node rooted at prp3
    HyperNode* prp3_node = new HyperNode(Dict::WID("PRP"), -1, make_pair(0,1));
    prp3_node->SetTrgSpan(src1_graph->GetNode(3)->GetTrgSpan());
    frags_exp.AddNode(prp3_node);
    // Node rooted at vp4
    HyperNode* vp4_node = new HyperNode(Dict::WID("VP"), -1, make_pair(1,4));
    vp4_node->SetTrgSpan(src1_graph->GetNode(5)->GetTrgSpan());
    frags_exp.AddNode(vp4_node);
    // Node rooted at vb7
    HyperNode* vb7_node = new HyperNode(Dict::WID("VB"), -1, make_pair(3,4));
    vb7_node->SetTrgSpan(src1_graph->GetNode(10)->GetTrgSpan());
    frags_exp.AddNode(vb7_node);
    // ---- Add edges ----
    // Edge for root0
    HyperEdge* root0_edge = new HyperEdge(root0_node);
    frags_exp.AddEdge(root0_edge);
    root0_edge->AddTail(s1_node);
    root0_edge->AddFragmentEdge(src1_graph->GetEdge(0));
    // root0_edge->AddTrgWord(-1);
    root0_node->AddEdge(root0_edge);
    // Edge for s1
    HyperEdge* s1_edge = new HyperEdge(s1_node);
    frags_exp.AddEdge(s1_edge);
    s1_edge->AddTail(np2_node);
    s1_edge->AddTail(vp4_node);
    s1_edge->AddFragmentEdge(src1_graph->GetEdge(1));
    // s1_edge->AddTrgWord(-1); s1_edge->AddTrgWord(-2);
    s1_node->AddEdge(s1_edge);
    // Edge for np2
    HyperEdge* np2_edge = new HyperEdge(np2_node);
    frags_exp.AddEdge(np2_edge);
    np2_edge->AddTail(prp3_node);
    np2_edge->AddFragmentEdge(src1_graph->GetEdge(2));
    // np2_edge->AddTrgWord(-1);
    np2_node->AddEdge(np2_edge);
    // Edge for prp3
    HyperEdge* prp3_edge = new HyperEdge(prp3_node);
    frags_exp.AddEdge(prp3_edge);
    prp3_edge->AddFragmentEdge(src1_graph->GetEdge(3));
    // prp3_edge->AddTrgWord(Dict::WID("il"));
    prp3_node->AddEdge(prp3_edge);
    // Edge for vp4
    HyperEdge* vp4_edge = new HyperEdge(vp4_node);
    frags_exp.AddEdge(vp4_edge);
    vp4_edge->AddTail(vb7_node);
    vp4_edge->AddFragmentEdge(src1_graph->GetEdge(4));
    vp4_edge->AddFragmentEdge(src1_graph->GetEdge(5));
    vp4_edge->AddFragmentEdge(src1_graph->GetEdge(6));
    // vp4_edge->AddTrgWord(Dict::WID("ne")); vp4_edge->AddTrgWord(-1); vp4_edge->AddTrgWord(Dict::WID("pas"));
    vp4_node->AddEdge(vp4_edge);
    // Edge for vb7
    HyperEdge* vb7_edge = new HyperEdge(vb7_node);
    frags_exp.AddEdge(vb7_edge);
    vb7_edge->AddFragmentEdge(src1_graph->GetEdge(7));
    // vb7_edge->AddTrgWord(Dict::WID("va"));
    vb7_node->AddEdge(vb7_edge);
    // Check to make sure that these are equal
    return frags_exp.CheckEqual(*frags_act);
}

int TestRuleExtractor::TestForestExtraction() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    src2_graph->GetEdge(0)->SetScore(log(0.5));
    src2_graph->GetEdge(1)->SetScore(log(0.5));
    boost::shared_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src2_graph, align2));
    // Create the actual values, all non-terminal edges should be fine, and most should have
    // a probability of 0.5, as they only belong to one of the two trees
    vector<double> scores_exp(9, 0.0), scores_act(9);
    for(int i = 0; i < 9; i++) 
        scores_act[i] = frags_act->GetEdge(i)->GetScore();
    // The only two edges that are 0.5 are the split edges on top
    scores_exp[0] = log(0.5); scores_exp[1] = log(0.5);
    // Check to make sure that these are equal
    return CheckAlmostVector(scores_exp, scores_act);
}

int TestRuleExtractor::TestForestExtractionBinarized() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::shared_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src4_graph, align4));
    ostringstream oss;
    // Printing also checks to make sure that there are no overlapping segments
    vector<string> rule_exp, rule_act;
    BOOST_FOREACH(HyperEdge* edge, frags_act->GetEdges())
        rule_act.push_back(forest_ext.RuleToString(*edge, src4_graph->GetWords(), trg4_sent, align4));
    std::string align4_str = "0-0 0-1 1-2 2-3 2-4 3-3 4-5";
    // This should cover all the rules extracted
    rule_exp.push_back("root ( x0:np ) ||| x0 ||| 1 ||| ");
    rule_exp.push_back("np ( x0:np x1:prn ) ||| x0 x1 ||| 1 ||| ");
    rule_exp.push_back("np ( x0:nnp ) ||| x0 ||| 1 ||| ");
    rule_exp.push_back("nnp ( \"landscape\" ) ||| \"sansui\" \"zu\" ||| 1 ||| 0-0 0-1");
    rule_exp.push_back("prn ( x0:-lrb- x1:prn' ) ||| x0 x1 ||| 1 ||| ");
    rule_exp.push_back("prn ( x0:prn' x1:-rrb- ) ||| x0 x1 ||| 1 ||| ");
    rule_exp.push_back("prn' ( x0:-lrb- x1:np ) ||| x0 x1 ||| 1 ||| ");
    rule_exp.push_back("prn' ( x0:np x1:-rrb- ) ||| x0 x1 ||| 1 ||| ");
    rule_exp.push_back("-lrb- ( \"-lrb-\" ) ||| \"(\" ||| 1 ||| 0-0");
    rule_exp.push_back("-rrb- ( \"-rrb-\" ) ||| \")\" ||| 1 ||| 0-0");
    rule_exp.push_back("np ( nnp ( \"private\" ) nnp ( \"collection\" ) ) ||| \"kojin\" \"zo\" ||| 1 ||| 0-0 0-1 1-0");
    sort(rule_exp.begin(), rule_exp.end());
    sort(rule_act.begin(), rule_act.end());
    return CheckVector(rule_exp, rule_act);
}

int TestRuleExtractor::TestTopNullExtraction() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::scoped_ptr<HyperGraph> frags_min(forest_ext.ExtractMinimalRules(*src3_graph, align3));
    boost::scoped_ptr<HyperGraph> frags_act(forest_ext.AttachNullsTop(*frags_min, align3, trg3_sent.size()));
    // Create the actual rule graph
    // Expected nodes "(ROOT0 (VP1 (VBD2 ate) (NP4 (NN5 rice))))";
    HyperGraph frags_exp;
    frags_exp.SetWords(src3_graph->GetWords());
    // ---- Add nodes ----
    // Node rooted at root0
    HyperNode* root0_node = new HyperNode(Dict::WID("ROOT"), -1, make_pair(0,2));
    root0_node->SetTrgSpan(src3_graph->GetNode(0)->GetTrgSpan());
    frags_exp.AddNode(root0_node);
    // Node rooted at vp1
    HyperNode* vp1_node = new HyperNode(Dict::WID("VP"), -1, make_pair(0,2));
    vp1_node->SetTrgSpan(src3_graph->GetNode(1)->GetTrgSpan());
    vp1_node->GetTrgSpan().insert(1);
    frags_exp.AddNode(vp1_node);
    // Node rooted at vbd2 without "o"
    HyperNode* vbd2_node = new HyperNode(Dict::WID("VBD"), -1, make_pair(0,1));
    vbd2_node->SetTrgSpan(src3_graph->GetNode(2)->GetTrgSpan());
    frags_exp.AddNode(vbd2_node);
    // Node rooted at np4 without "o"
    HyperNode* np4_node = new HyperNode(Dict::WID("NP"), -1, make_pair(1,2));
    np4_node->SetTrgSpan(src3_graph->GetNode(4)->GetTrgSpan());
    frags_exp.AddNode(np4_node);
    // Node rooted at nn5 without "o"
    HyperNode* nn5_node = new HyperNode(Dict::WID("NN"), -1, make_pair(1,2));
    nn5_node->SetTrgSpan(src3_graph->GetNode(5)->GetTrgSpan());
    frags_exp.AddNode(nn5_node);
    // ---- Add edges ----
    // Edge for root0
    HyperEdge* root0_edge = new HyperEdge(root0_node);
    frags_exp.AddEdge(root0_edge);
    root0_edge->AddTail(vp1_node);
    root0_edge->AddFragmentEdge(src1_graph->GetEdge(0));
    root0_node->AddEdge(root0_edge);
    // Edge for vp1
    HyperEdge* vp1_edge = new HyperEdge(vp1_node);
    frags_exp.AddEdge(vp1_edge);
    vp1_edge->AddTail(vbd2_node);
    vp1_edge->AddTail(np4_node);
    vp1_edge->AddFragmentEdge(src1_graph->GetEdge(1));
    vp1_node->AddEdge(vp1_edge);
    // Edge for vbd2
    HyperEdge* vbd2_edge = new HyperEdge(vbd2_node);
    frags_exp.AddEdge(vbd2_edge);
    vbd2_edge->AddFragmentEdge(src1_graph->GetEdge(2));
    vbd2_node->AddEdge(vbd2_edge);
    // Edge for np4
    HyperEdge* np4_edge = new HyperEdge(np4_node);
    frags_exp.AddEdge(np4_edge);
    np4_edge->AddTail(nn5_node);
    np4_edge->AddFragmentEdge(src1_graph->GetEdge(3));
    np4_node->AddEdge(np4_edge);
    // Edge for nn5
    HyperEdge* nn5_edge = new HyperEdge(nn5_node);
    frags_exp.AddEdge(nn5_edge);
    nn5_edge->AddFragmentEdge(src1_graph->GetEdge(4));
    nn5_node->AddEdge(nn5_edge);
    return frags_exp.CheckEqual(*frags_act);
}

int TestRuleExtractor::TestExpandNode() {
    ForestExtractor forest_ext;
    HyperNode old_node(1, -1, make_pair(5,6), 1);
    old_node.GetTrgSpan().insert(3);
    vector<bool> nulls(7, true);
    nulls[1] = false; nulls[3] = false; nulls[5] = false;
    // Get the actual list of expanded nodes
    ForestExtractor::SpanNodeVector sn_act = forest_ext.ExpandNode(nulls, old_node, 1);
    // Make the expected list of expanded nodes
    vector<HyperNode*> node_exp, node_act;
    vector<set<int> > set_exp, set_act;
    for(int i = 0; i < 4; i++) {
        set_act.push_back(sn_act[i].first);
        node_act.push_back(sn_act[i].second);
        set_exp.push_back(set<int>());
        HyperNode * next_node = new HyperNode(old_node);
        next_node->SetId(-1);
        node_exp.push_back(next_node);
    }
    node_exp[1]->GetTrgSpan().insert(4);
    set_exp[1].insert(4);
    node_exp[2]->GetTrgSpan().insert(2);
    set_exp[2].insert(2);
    node_exp[3]->GetTrgSpan().insert(2); node_exp[3]->GetTrgSpan().insert(4);
    set_exp[3].insert(2); set_exp[3].insert(4);
    bool ret = CheckVector(set_exp, set_act) && CheckPtrVector(node_exp, node_act); 
    BOOST_FOREACH(ForestExtractor::SpanNodeVector::value_type val, sn_act)
        delete val.second;
    BOOST_FOREACH(HyperNode * ptr, node_exp)
        delete ptr;
    return ret;
}

int TestRuleExtractor::TestExhaustiveNullExtraction() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::scoped_ptr<HyperGraph> frags_min(forest_ext.ExtractMinimalRules(*src3_graph, align3));
    boost::scoped_ptr<HyperGraph> frags_act(forest_ext.AttachNullsExhaustive(*frags_min, align3, trg3_sent.size()));
    // Create the actual rule graph
    // Expected nodes "(ROOT0 (VP1 (VBD2 ate) (NP4 (NN5 rice))))";
    HyperGraph frags_exp;
    frags_exp.SetWords(src3_graph->GetWords());
    // ---- Add nodes ----
    // Node rooted at pseudo-root
    HyperNode* pseudo_node = new HyperNode;
    frags_exp.AddNode(pseudo_node);
    pseudo_node->SetFrontier(HyperNode::NOT_FRONTIER);
    // Node rooted at root0
    HyperNode* root0_node = new HyperNode(Dict::WID("ROOT"), -1, make_pair(0,2));
    root0_node->SetTrgSpan(src3_graph->GetNode(0)->GetTrgSpan());
    frags_exp.AddNode(root0_node);
    // Node rooted at vp1
    HyperNode* vp1_node = new HyperNode(Dict::WID("VP"), -1, make_pair(0,2));
    vp1_node->SetTrgSpan(src3_graph->GetNode(1)->GetTrgSpan());
    frags_exp.AddNode(vp1_node);
    // Node rooted at vbd2 without "o"
    HyperNode* vbd2_node_n = new HyperNode(Dict::WID("VBD"), -1, make_pair(0,1));
    vbd2_node_n->SetTrgSpan(src3_graph->GetNode(2)->GetTrgSpan());
    frags_exp.AddNode(vbd2_node_n);
    // Node rooted at vbd2 with "o"
    HyperNode* vbd2_node_y = new HyperNode(Dict::WID("VBD"), -1, make_pair(0,1));
    vbd2_node_y->SetTrgSpan(src3_graph->GetNode(2)->GetTrgSpan());
    vbd2_node_y->GetTrgSpan().insert(1);
    frags_exp.AddNode(vbd2_node_y);
    // Node rooted at np4 without "o"
    HyperNode* np4_node_n = new HyperNode(Dict::WID("NP"), -1, make_pair(1,2));
    np4_node_n->SetTrgSpan(src3_graph->GetNode(4)->GetTrgSpan());
    frags_exp.AddNode(np4_node_n);
    // Node rooted at np4 with "o"
    HyperNode* np4_node_y = new HyperNode(Dict::WID("NP"), -1, make_pair(1,2));
    np4_node_y->SetTrgSpan(src3_graph->GetNode(4)->GetTrgSpan());
    np4_node_y->GetTrgSpan().insert(1);
    frags_exp.AddNode(np4_node_y);
    // Node rooted at nn5 without "o"
    HyperNode* nn5_node_n = new HyperNode(Dict::WID("NN"), -1, make_pair(1,2));
    nn5_node_n->SetTrgSpan(src3_graph->GetNode(5)->GetTrgSpan());
    frags_exp.AddNode(nn5_node_n);
    // Node rooted at nn5 with "o"
    HyperNode* nn5_node_y = new HyperNode(Dict::WID("NN"), -1, make_pair(1,2));
    nn5_node_y->SetTrgSpan(src3_graph->GetNode(5)->GetTrgSpan());
    nn5_node_y->GetTrgSpan().insert(1);
    frags_exp.AddNode(nn5_node_y);
    // ---- Add edges ----
    // Edge for pseudo-node
    HyperEdge* p_r0 = new HyperEdge(pseudo_node);
    frags_exp.AddEdge(p_r0);
    p_r0->AddTail(root0_node);
    pseudo_node->AddEdge(p_r0);
    // Edge for root0
    HyperEdge* r0_v1 = new HyperEdge(root0_node);
    frags_exp.AddEdge(r0_v1);
    r0_v1->AddTail(vp1_node);
    r0_v1->AddFragmentEdge(src1_graph->GetEdge(0));
    root0_node->AddEdge(r0_v1);
    // Edge for vp1 with no attachment
    HyperEdge* v1_v2n_n4n = new HyperEdge(vp1_node);
    frags_exp.AddEdge(v1_v2n_n4n);
    v1_v2n_n4n->AddTail(vbd2_node_n);
    v1_v2n_n4n->AddTail(np4_node_n);
    v1_v2n_n4n->AddFragmentEdge(src1_graph->GetEdge(1));
    vp1_node->AddEdge(v1_v2n_n4n);
    // Edge for vp1 with right attached
    HyperEdge* v1_v2n_n4y = new HyperEdge(vp1_node);
    frags_exp.AddEdge(v1_v2n_n4y);
    v1_v2n_n4y->AddTail(vbd2_node_n);
    v1_v2n_n4y->AddTail(np4_node_y);
    v1_v2n_n4y->AddFragmentEdge(src1_graph->GetEdge(1));
    vp1_node->AddEdge(v1_v2n_n4y);
    // Edge for vp1 with left attached
    HyperEdge* v1_v2y_n4n = new HyperEdge(vp1_node);
    frags_exp.AddEdge(v1_v2y_n4n);
    v1_v2y_n4n->AddTail(vbd2_node_y);
    v1_v2y_n4n->AddTail(np4_node_n);
    v1_v2y_n4n->AddFragmentEdge(src1_graph->GetEdge(1));
    vp1_node->AddEdge(v1_v2y_n4n);
    // Edge for vbd2 without "o"
    HyperEdge* vbd2_edge_n = new HyperEdge(vbd2_node_n);
    frags_exp.AddEdge(vbd2_edge_n);
    vbd2_edge_n->AddFragmentEdge(src1_graph->GetEdge(2));
    vbd2_node_n->AddEdge(vbd2_edge_n);
    // Edge for vbd2 with "o"
    HyperEdge* vbd2_edge_y = new HyperEdge(vbd2_node_y);
    frags_exp.AddEdge(vbd2_edge_y);
    vbd2_edge_y->AddFragmentEdge(src1_graph->GetEdge(2));
    vbd2_node_y->AddEdge(vbd2_edge_y);
    // Edge for np4
    HyperEdge* np4_edge_nn = new HyperEdge(np4_node_n);
    frags_exp.AddEdge(np4_edge_nn);
    np4_edge_nn->AddTail(nn5_node_n);
    np4_edge_nn->AddFragmentEdge(src1_graph->GetEdge(3));
    np4_node_n->AddEdge(np4_edge_nn);
    // Edge for np4
    HyperEdge* np4_edge_yn = new HyperEdge(np4_node_y);
    frags_exp.AddEdge(np4_edge_yn);
    np4_edge_yn->AddTail(nn5_node_n);
    np4_edge_yn->AddFragmentEdge(src1_graph->GetEdge(3));
    np4_node_y->AddEdge(np4_edge_yn);
    // Edge for np4
    HyperEdge* np4_edge_yy = new HyperEdge(np4_node_y);
    frags_exp.AddEdge(np4_edge_yy);
    np4_edge_yy->AddTail(nn5_node_y);
    np4_edge_yy->AddFragmentEdge(src1_graph->GetEdge(3));
    np4_node_y->AddEdge(np4_edge_yy);
    // Edge for nn5 without "o"
    HyperEdge* nn5_edge_n = new HyperEdge(nn5_node_n);
    frags_exp.AddEdge(nn5_edge_n);
    nn5_edge_n->AddFragmentEdge(src1_graph->GetEdge(4));
    nn5_node_n->AddEdge(nn5_edge_n);
    // Edge for nn5 with "o"
    HyperEdge* nn5_edge_y = new HyperEdge(nn5_node_y);
    frags_exp.AddEdge(nn5_edge_y);
    nn5_edge_y->AddFragmentEdge(src1_graph->GetEdge(4));
    nn5_node_y->AddEdge(nn5_edge_y);
    return frags_exp.CheckEqual(*frags_act);
}

int TestRuleExtractor::TestExhaustiveNullDisconnected() {
    std::string in_str =
"{\"nodes\": ["
    "{\"sym\": \"root\", \"span\": [0, 11], \"id\": 0, \"edges\": [0], \"trg_span\": [0, 2, 3, 4, 5, 6, 7, 9, 12, 13]},"
    "{\"sym\": \"s\", \"span\": [0, 11], \"id\": 1, \"edges\": [1], \"trg_span\": [0, 2, 3, 4, 5, 6, 7, 9, 12, 13]},"
    "{\"sym\": \".\", \"span\": [10, 11], \"id\": 2, \"edges\": [2], \"trg_span\": [12, 13]},"
    "{\"sym\": \"s'\", \"span\": [0, 10], \"id\": 3, \"edges\": [3], \"trg_span\": [0, 2, 3, 4, 5, 6, 7, 9]},"
    "{\"sym\": \"vp\", \"span\": [4, 10], \"id\": 4, \"edges\": [4], \"trg_span\": [3, 4, 5, 6, 7]},"
    "{\"sym\": \"np\", \"span\": [3, 4], \"id\": 5, \"edges\": [5], \"trg_span\": [2]},"
    "{\"sym\": \"nn\", \"span\": [2, 3], \"id\": 6, \"edges\": [6], \"trg_span\": [0]},"
    "{\"sym\": \"dt\", \"span\": [1, 2], \"id\": 7, \"edges\": [7], \"trg_span\": [9]},"
    "{\"sym\": \"prp\", \"span\": [3, 4], \"id\": 8, \"edges\": [8], \"trg_span\": [2]},"
    "{\"sym\": \"vbz\", \"span\": [4, 5], \"id\": 9, \"edges\": [9], \"trg_span\": [7]},"
    "{\"sym\": \"pp\", \"span\": [7, 10], \"id\": 10, \"edges\": [10], \"trg_span\": [3]},"
    "{\"sym\": \"np\", \"span\": [8, 10], \"id\": 11, \"edges\": [11], \"trg_span\": [3]},"
    "{\"sym\": \"dt\", \"span\": [8, 9], \"id\": 12, \"edges\": [12], \"trg_span\": [3]}"
"], \"edges\": ["
    "{\"id\": 0, \"head\": 0, \"tails\": [1]},"
    "{\"id\": 1, \"head\": 1, \"tails\": [3, 2]},"
    "{\"id\": 2, \"head\": 2},"
    "{\"id\": 3, \"head\": 3, \"tails\": [4, 5, 7, 6]},"
    "{\"id\": 4, \"head\": 4, \"tails\": [9, 10]},"
    "{\"id\": 5, \"head\": 5, \"tails\": [8]},"
    "{\"id\": 6, \"head\": 6},"
    "{\"id\": 7, \"head\": 7},"
    "{\"id\": 8, \"head\": 8},"
    "{\"id\": 9, \"head\": 9},"
    "{\"id\": 10, \"head\": 10, \"tails\": [11]},"
    "{\"id\": 11, \"head\": 11, \"tails\": [12]},"
    "{\"id\": 12, \"head\": 12}"
"], \"words\": ["
    "\"for\","
    "\"that\","
    "\"reason\","
    "\"it\","
    "\"is\","
    "\"not\","
    "\"open\","
    "\"to\","
    "\"the\","
    "\"public\","
    "\".\""
"]}";
    std::string align_str = "1-9 2-0 3-2 4-7 5-4 6-4 6-5 6-6 8-3 10-12 10-13";
    Alignment align = Alignment::FromString(align_str);
    istringstream in1(in_str);
    JSONTreeIO io;
    boost::shared_ptr<HyperGraph> hg_in(io.ReadTree(in1));
    ForestExtractor forest_ext;
    forest_ext.SetMaxNonterm(3);
    boost::shared_ptr<HyperGraph> hg_out(forest_ext.AttachNullsExhaustive(*hg_in, align, 14));
    hg_out->InsideOutsideNormalize(); // We want to check that it doesn't die here
    return 1;
}

int TestRuleExtractor::TestRulePrinting() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::shared_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src1_graph, align1));
    vector<string> rule_exp, rule_act;
    // Get actual rules, plus one composed rule
    BOOST_FOREACH(HyperEdge* edge, frags_act->GetEdges())
        rule_act.push_back(forest_ext.RuleToString(*edge, src1_graph->GetWords(), trg1_sent, align1));
    boost::shared_ptr<HyperEdge> e01(RuleComposer::ComposeEdge(*frags_act->GetEdge(0), *frags_act->GetEdge(1), 0));
    rule_act.push_back(forest_ext.RuleToString(*e01, src1_graph->GetWords(), trg1_sent, align1));
    rule_exp.push_back("ROOT ( x0:S ) ||| x0 ||| 1 ||| ");
    rule_exp.push_back("S ( x0:NP x1:VP ) ||| x0 x1 ||| 1 ||| ");
    rule_exp.push_back("NP ( x0:PRP ) ||| x0 ||| 1 ||| ");
    rule_exp.push_back("PRP ( \"he\" ) ||| \"il\" ||| 1 ||| 0-0");
    rule_exp.push_back("VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB ) ||| \"ne\" x0 \"pas\" ||| 1 ||| 0-0 0-1 1-0 1-1");
    rule_exp.push_back("VB ( \"go\" ) ||| \"va\" ||| 1 ||| 0-0");
    rule_exp.push_back("ROOT ( S ( x0:NP x1:VP ) ) ||| x0 x1 ||| 1 ||| ");
    sort(rule_exp.begin(), rule_exp.end());
    sort(rule_act.begin(), rule_act.end());
    return CheckVector(rule_exp, rule_act);
}

int TestRuleExtractor::TestRulePrintingTrgSyntax() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::shared_ptr<HyperGraph> frags_act(forest_ext.ExtractMinimalRules(*src1_graph, align1));
    vector<string> rule_exp, rule_act;
    LabeledSpans trg1_spans = trg1_graph->GetLabeledSpans();
    // Get actual rules, plus one composed rule
    BOOST_FOREACH(HyperEdge* edge, frags_act->GetEdges())
        rule_act.push_back(forest_ext.RuleToString(*edge, src1_graph->GetWords(), trg1_sent, align1, &trg1_spans));
    boost::shared_ptr<HyperEdge> e01(RuleComposer::ComposeEdge(*frags_act->GetEdge(0), *frags_act->GetEdge(1), 0));
    rule_act.push_back(forest_ext.RuleToString(*e01, src1_graph->GetWords(), trg1_sent, align1, &trg1_spans));
    rule_exp.push_back("ROOT ( x0:S ) ||| x0:ROOT @ ROOT ||| 1 ||| ");
    rule_exp.push_back("S ( x0:NP x1:VP ) ||| x0:NP x1:VP @ ROOT ||| 1 ||| ");
    rule_exp.push_back("NP ( x0:PRP ) ||| x0:NP @ NP ||| 1 ||| ");
    rule_exp.push_back("PRP ( \"he\" ) ||| \"il\" @ NP ||| 1 ||| 0-0");
    rule_exp.push_back("VP ( AUX ( \"does\" ) RB ( \"not\" ) x0:VB ) ||| \"ne\" x0:VB \"pas\" @ VP ||| 1 ||| 0-0 0-1 1-0 1-1");
    rule_exp.push_back("VB ( \"go\" ) ||| \"va\" @ VB ||| 1 ||| 0-0");
    rule_exp.push_back("ROOT ( S ( x0:NP x1:VP ) ) ||| x0:NP x1:VP @ ROOT ||| 1 ||| ");
    sort(rule_exp.begin(), rule_exp.end());
    sort(rule_act.begin(), rule_act.end());
    return CheckVector(rule_exp, rule_act);
}


int TestRuleExtractor::TestComposeEdge() {
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::scoped_ptr<HyperGraph> rule_graph(forest_ext.ExtractMinimalRules(*src1_graph, align1));
    const vector<HyperEdge*> & edges = rule_graph->GetEdges();
    const vector<HyperNode*> & nodes = rule_graph->GetNodes();
    // Compose
    // Expected node numbers: "(ROOT0 (S1 (NP2 (PRP3 he)) (VP4 (AUX5 does) (RB6 not) (VB7 go))))";
    // Compose the edge over S1 and VP4
    boost::shared_ptr<HyperEdge> act14(RuleComposer::ComposeEdge(*edges[1], *edges[4], 1));
    // Build the expected edge
    boost::shared_ptr<HyperEdge> exp14(new HyperEdge(nodes[1]));
    exp14->AddTail(nodes[2]); exp14->AddTail(nodes[5]); 
    // Score is not tested yet
    // exp14->SetScore(-0.4);
    // Not clear if calculating rule strings is necessary, as rules are generally
    // composed before calculating their string representations
    // exp14->SetRuleStr("S ( V ( \"a\" ) x0:N )");
    BOOST_FOREACH(const SparsePair & kv, edges[0]->GetFeatures().GetImpl()) exp14->GetFeatures().Add(kv.first, kv.second);
    BOOST_FOREACH(const SparsePair & kv, edges[2]->GetFeatures().GetImpl()) exp14->GetFeatures().Add(kv.first, kv.second);
    // Don't add these, as target words are not extracted
    // exp14->AddTrgWord(-1); exp14->AddTrgWord(Dict::WID("ne")); exp14->AddTrgWord(-2); exp14->AddTrgWord(Dict::WID("pas"));
    exp14->AddFragmentEdge(src1_graph->GetEdge(1));
    exp14->AddFragmentEdge(src1_graph->GetEdge(4));
    exp14->AddFragmentEdge(src1_graph->GetEdge(5));
    exp14->AddFragmentEdge(src1_graph->GetEdge(6));
    return CheckEqual(*exp14, *act14);
}

int TestRuleExtractor::TestRuleComposer() {
    ForestExtractor forest_ext;
    boost::scoped_ptr<HyperGraph> rule_graph(forest_ext.ExtractMinimalRules(*src1_graph, align1));
    RuleComposer rc(2);
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph(*rule_graph)), act_graph(rc.TransformGraph(*rule_graph));
    // Example rule graph
    const vector<HyperEdge*> & edges = exp_graph->GetEdges();

    // Make composed rules
    HyperEdge * e23 = RuleComposer::ComposeEdge(*edges[2], *edges[3], 0); exp_graph->AddEdge(e23); exp_graph->GetNode(2)->AddEdge(e23);
    HyperEdge * e12 = RuleComposer::ComposeEdge(*edges[1], *edges[2], 0); exp_graph->AddEdge(e12); exp_graph->GetNode(1)->AddEdge(e12);
    HyperEdge * e45 = RuleComposer::ComposeEdge(*edges[4], *edges[5], 0); exp_graph->AddEdge(e45); exp_graph->GetNode(4)->AddEdge(e45);
    HyperEdge * e14 = RuleComposer::ComposeEdge(*edges[1], *edges[4], 1); exp_graph->AddEdge(e14); exp_graph->GetNode(1)->AddEdge(e14);
    HyperEdge * e02 = RuleComposer::ComposeEdge(*edges[0], *edges[1], 0); exp_graph->AddEdge(e02); exp_graph->GetNode(0)->AddEdge(e02);

    return exp_graph->CheckEqual(*act_graph);
}

int TestRuleExtractor::TestRuleComposerLex() {
    ForestExtractor forest_ext;
    boost::scoped_ptr<HyperGraph> rule_graph(forest_ext.ExtractMinimalRules(*src1_graph, align1));
    RuleComposer rc(1, 3);
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph(*rule_graph)), act_graph(rc.TransformGraph(*rule_graph));
    // Example rule graph
    const vector<HyperEdge*> & edges = exp_graph->GetEdges();

    // Make lexicalized composed rules of size up to 3
    HyperEdge * e23 = RuleComposer::ComposeEdge(*edges[2], *edges[3], 0); exp_graph->AddEdge(e23); exp_graph->GetNode(2)->AddEdge(e23);
    HyperEdge * e45 = RuleComposer::ComposeEdge(*edges[4], *edges[5], 0); exp_graph->AddEdge(e45); exp_graph->GetNode(4)->AddEdge(e45);

    return exp_graph->CheckEqual(*act_graph);
}

int TestRuleExtractor::TestTrinary() {
    // Use the example from Galley et al.
    std::string src_tree = "(S (A a) (B b))";
    std::string trg_str  = "a b";
    std::string align_str = "0-0 1-1";
    istringstream iss(src_tree);
    boost::shared_ptr<HyperGraph> src_hg(tree_io.ReadTree(iss));
    Sentence trg_sent = Dict::ParseWords(trg_str);
    Alignment align = Alignment::FromString(align_str);
    // Run the Forest algorithm
    ForestExtractor forest_ext;
    boost::shared_ptr<HyperGraph> rule_hg(forest_ext.ExtractMinimalRules(*src_hg, align));
    // Compose
    RuleComposer rc(3);
    boost::shared_ptr<HyperGraph> act_hg(rc.TransformGraph(*rule_hg));
    // Get actual rules, plus one composed rule
    vector<string> rule_exp, rule_act;
    BOOST_FOREACH(HyperEdge* edge, act_hg->GetEdges())
        rule_act.push_back(forest_ext.RuleToString(*edge, src_hg->GetWords(), trg_sent, align));
    rule_exp.push_back("A ( \"a\" ) ||| \"a\" ||| 1 ||| 0-0");
    rule_exp.push_back("B ( \"b\" ) ||| \"b\" ||| 1 ||| 0-0");
    rule_exp.push_back("S ( A ( \"a\" ) B ( \"b\" ) ) ||| \"a\" \"b\" ||| 1 ||| 0-0 1-1");
    rule_exp.push_back("S ( A ( \"a\" ) x0:B ) ||| \"a\" x0 ||| 1 ||| 0-0");
    rule_exp.push_back("S ( x0:A B ( \"b\" ) ) ||| x0 \"b\" ||| 1 ||| 0-0");
    rule_exp.push_back("S ( x0:A x1:B ) ||| x0 x1 ||| 1 ||| ");
    sort(rule_exp.begin(), rule_exp.end());
    sort(rule_act.begin(), rule_act.end());
    return CheckVector(rule_exp, rule_act);
}

bool TestRuleExtractor::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestTreeExtraction()" << endl; if(TestTreeExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestForestExtraction()" << endl; if(TestForestExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestForestExtractionBinarized()" << endl; if(TestForestExtractionBinarized()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestExpandNode()" << endl; if(TestExpandNode()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestTopNullExtraction()" << endl; if(TestTopNullExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestExhaustiveNullExtraction()" << endl; if(TestExhaustiveNullExtraction()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestExhaustiveNullDisconnected()" << endl; if(TestExhaustiveNullDisconnected()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRulePrinting()" << endl; if(TestRulePrinting()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRulePrintingTrgSyntax()" << endl; if(TestRulePrintingTrgSyntax()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestComposeEdge()" << endl; if(TestComposeEdge()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRuleComposer()" << endl; if(TestRuleComposer()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestRuleComposerLex()" << endl; if(TestRuleComposerLex()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestTrinary()" << endl; if(TestTrinary()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestRuleExtractor Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

