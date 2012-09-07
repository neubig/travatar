#ifndef TEST_BINARIZER_H__
#define TEST_BINARIZER_H__

#include "test-base.h"
#include <travatar/hyper-graph.h>
#include <travatar/binarizer-directional.h>
#include <travatar/binarizer-cky.h>

using namespace boost;

namespace travatar {

class TestBinarizer : public TestBase {

public:

    TestBinarizer() {
        // Example rule graph
        trinary_graph_.reset(new HyperGraph);
        src_.resize(3); src_[0] = Dict::WID("s"); src_[1] = Dict::WID("t"); src_[2] = Dict::WID("u");
        trinary_graph_->SetWords(src_);
        HyperNode * na = new HyperNode; na->SetSpan(MakePair(0,3));   trinary_graph_->AddNode(na);  na->SetSym( Dict::WID("A" ));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(MakePair(0,1)); trinary_graph_->AddNode(nb1); nb1->SetSym(Dict::WID("B1"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(MakePair(1,2)); trinary_graph_->AddNode(nb2); nb2->SetSym(Dict::WID("B2"));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(MakePair(2,3)); trinary_graph_->AddNode(nb3); nb3->SetSym(Dict::WID("B3"));
        HyperEdge * e = new HyperEdge(na); trinary_graph_->AddEdge(e); e->AddTail(nb1); e->AddTail(nb2); e->AddTail(nb3); na->AddEdge(e);
        e->SetScore(1); e->AddFeature(Dict::WID("feat"), 1);
    }

    ~TestBinarizer() { }

    int TestBinarizerRight() {
        BinarizerDirectional br(BinarizerDirectional::BINARIZE_RIGHT);
        shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
        shared_ptr<HyperGraph> exp_graph(new HyperGraph);
        exp_graph->SetWords(src_);
        HyperNode * na = new HyperNode; na->SetSpan(MakePair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(MakePair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
        HyperNode * nb23 = new HyperNode; nb23->SetSpan(MakePair(1,3)); exp_graph->AddNode(nb23); nb23->SetSym(Dict::WID("A'"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(MakePair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(MakePair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("B3"));
        HyperEdge * e1 = new HyperEdge(na); exp_graph->AddEdge(e1); e1->AddTail(nb1); e1->AddTail(nb23); na->AddEdge(e1);
        e1->SetScore(1); e1->AddFeature(Dict::WID("feat"), 1);
        HyperEdge * e2 = new HyperEdge(nb23); exp_graph->AddEdge(e2); e2->AddTail(nb2); e2->AddTail(nb3); nb23->AddEdge(e2);
        return exp_graph->CheckEqual(*act_graph);
    }

    int TestBinarizerLeft() {
        BinarizerDirectional br(BinarizerDirectional::BINARIZE_LEFT);
        shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
        shared_ptr<HyperGraph> exp_graph(new HyperGraph);
        exp_graph->SetWords(src_);
        HyperNode * na = new HyperNode; na->SetSpan(MakePair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(MakePair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("B3"));
        HyperNode * nb12 = new HyperNode; nb12->SetSpan(MakePair(0,2)); exp_graph->AddNode(nb12); nb12->SetSym(Dict::WID("A'"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(MakePair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(MakePair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
        HyperEdge * e1 = new HyperEdge(na); exp_graph->AddEdge(e1); e1->AddTail(nb12); e1->AddTail(nb3); na->AddEdge(e1);
        e1->SetScore(1); e1->AddFeature(Dict::WID("feat"), 1);
        HyperEdge * e2 = new HyperEdge(nb12); exp_graph->AddEdge(e2); e2->AddTail(nb1); e2->AddTail(nb2); nb12->AddEdge(e2);
        return exp_graph->CheckEqual(*act_graph);
    }
    
    int TestBinarizerCKY() {
        BinarizerCKY br;
        shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
        shared_ptr<HyperGraph> exp_graph(new HyperGraph);
        exp_graph->SetWords(src_);
        // The order starts from 0-1, 0-2, 1-2, so first will be 12->1 2, na->1 23, na->12 3
        HyperNode * nb12 = new HyperNode; nb12->SetSpan(MakePair(0,2)); exp_graph->AddNode(nb12); nb12->SetSym(Dict::WID("A'"));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(MakePair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(MakePair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
        HyperNode * na = new HyperNode; na->SetSpan(MakePair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
        HyperNode * nb23 = new HyperNode; nb23->SetSpan(MakePair(1,3)); exp_graph->AddNode(nb23); nb23->SetSym(Dict::WID("A'"));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(MakePair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("B3"));
        HyperEdge * e12 = new HyperEdge(nb12); exp_graph->AddEdge(e12); e12->AddTail(nb1); e12->AddTail(nb2); nb12->AddEdge(e12);
        HyperEdge * e1r = new HyperEdge(na); exp_graph->AddEdge(e1r); e1r->AddTail(nb1); e1r->AddTail(nb23); na->AddEdge(e1r);
        HyperEdge * e1l = new HyperEdge(na); exp_graph->AddEdge(e1l); e1l->AddTail(nb12); e1l->AddTail(nb3); na->AddEdge(e1l);
        HyperEdge * e23 = new HyperEdge(nb23); exp_graph->AddEdge(e23); e23->AddTail(nb2); e23->AddTail(nb3); nb23->AddEdge(e23);
        e1l->SetScore(1); e1l->AddFeature(Dict::WID("feat"), 1);
        e1r->SetScore(1); e1r->AddFeature(Dict::WID("feat"), 1);
        return exp_graph->CheckEqual(*act_graph);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestBinarizerRight()" << endl; if(TestBinarizerRight()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestBinarizerLeft()" << endl; if(TestBinarizerLeft()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestBinarizerCKY()" << endl; if(TestBinarizerCKY()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestBinarizer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    PennTreeIO tree_io;
    boost::scoped_ptr<HyperGraph> trinary_graph_;
    boost::scoped_ptr<TranslationRule> rule_a, rule_b, rule_x, rule_y, rule_unk, rule_01, rule_10;
    vector<WordId> src_;

};

}

#endif
