#include "test-binarizer.h"

namespace travatar {

TestBinarizer::TestBinarizer() {
    // Example rule graph
    trinary_graph_.reset(new HyperGraph);
    {
        src_.resize(3); src_[0] = Dict::WID("s"); src_[1] = Dict::WID("t"); src_[2] = Dict::WID("u");
        trinary_graph_->SetWords(src_);
        HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));   trinary_graph_->AddNode(na);  na->SetSym( Dict::WID("A" ));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); trinary_graph_->AddNode(nb1); nb1->SetSym(Dict::WID("B1"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); trinary_graph_->AddNode(nb2); nb2->SetSym(Dict::WID("B2"));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); trinary_graph_->AddNode(nb3); nb3->SetSym(Dict::WID("."));
        HyperEdge * e = new HyperEdge(na); trinary_graph_->AddEdge(e); e->AddTail(nb1); e->AddTail(nb2); e->AddTail(nb3); na->AddEdge(e);
        e->SetScore(1); e->AddFeature(Dict::WID("feat"), 1);
    }
    // An example of a unordered graph with an intervening edge
    unordered_graph_.reset(new HyperGraph);
    {
        src_.resize(3); src_[0] = Dict::WID("s"); src_[1] = Dict::WID("t"); src_[2] = Dict::WID("u");
        unordered_graph_->SetWords(src_);
        HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));   unordered_graph_->AddNode(na);  na->SetSym( Dict::WID("A" ));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); unordered_graph_->AddNode(nb1); nb1->SetSym(Dict::WID("B1"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); unordered_graph_->AddNode(nb2); nb2->SetSym(Dict::WID("B2"));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); unordered_graph_->AddNode(nb3); nb3->SetSym(Dict::WID("."));
        HyperEdge * e1 = new HyperEdge(nb1); unordered_graph_->AddEdge(e1); nb1->AddEdge(e1);
        HyperEdge * e = new HyperEdge(na); unordered_graph_->AddEdge(e); e->AddTail(nb1); e->AddTail(nb2); e->AddTail(nb3); na->AddEdge(e);
        e->SetScore(1); e->AddFeature(Dict::WID("feat"), 1);
    }
    // Example rule graph
    double_graph_.reset(new HyperGraph);
    {
        src_.resize(3); src_[0] = Dict::WID("s"); src_[1] = Dict::WID("t"); src_[2] = Dict::WID("u");
        double_graph_->SetWords(src_);
        HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));   double_graph_->AddNode(na);  na->SetSym( Dict::WID("A" ));
        HyperNode * nrx = new HyperNode; nrx->SetSpan(make_pair(0,3)); double_graph_->AddNode(nrx); nrx->SetSym(Dict::WID("X" ));
        HyperNode * nry = new HyperNode; nry->SetSpan(make_pair(0,3)); double_graph_->AddNode(nry); nry->SetSym(Dict::WID("Y" ));
        HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); double_graph_->AddNode(nb1); nb1->SetSym(Dict::WID("B1"));
        HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); double_graph_->AddNode(nb2); nb2->SetSym(Dict::WID("B2"));
        HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); double_graph_->AddNode(nb3); nb3->SetSym(Dict::WID("."));
        HyperEdge * eax = new HyperEdge(na);  double_graph_->AddEdge(eax); eax->AddTail(nrx); nrx->AddEdge(eax);
        HyperEdge * eay = new HyperEdge(na);  double_graph_->AddEdge(eay); eay->AddTail(nry); nrx->AddEdge(eay);
        HyperEdge * erx = new HyperEdge(nrx); double_graph_->AddEdge(erx); erx->AddTail(nb1); erx->AddTail(nb2); erx->AddTail(nb3); nrx->AddEdge(erx);
        HyperEdge * ery = new HyperEdge(nry); double_graph_->AddEdge(ery); ery->AddTail(nb1); ery->AddTail(nb2); ery->AddTail(nb3); nry->AddEdge(ery);
        eax->SetScore(1); eax->AddFeature(Dict::WID("feat"), 1);
        eay->SetScore(1); eay->AddFeature(Dict::WID("feat"), 1);
    }
}

TestBinarizer::~TestBinarizer() {}

int TestBinarizer::TestBinarizerRight() {
    BinarizerDirectional br(BinarizerDirectional::BINARIZE_RIGHT);
    boost::shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    exp_graph->SetWords(src_);
    HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
    HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
    HyperNode * nb23 = new HyperNode; nb23->SetSpan(make_pair(1,3)); exp_graph->AddNode(nb23); nb23->SetSym(Dict::WID("A'"));
    HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
    HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("."));
    HyperEdge * e1 = new HyperEdge(na); exp_graph->AddEdge(e1); e1->AddTail(nb1); e1->AddTail(nb23); na->AddEdge(e1);
    e1->SetScore(1); e1->AddFeature(Dict::WID("feat"), 1);
    HyperEdge * e2 = new HyperEdge(nb23); exp_graph->AddEdge(e2); e2->AddTail(nb2); e2->AddTail(nb3); nb23->AddEdge(e2);
    return exp_graph->CheckEqual(*act_graph);
}

int TestBinarizer::TestBinarizerRightRaisePunc() {
    BinarizerDirectional br(BinarizerDirectional::BINARIZE_RIGHT, true);
    boost::shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    exp_graph->SetWords(src_);
    HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
    HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("."));
    HyperNode * nb12 = new HyperNode; nb12->SetSpan(make_pair(0,2)); exp_graph->AddNode(nb12); nb12->SetSym(Dict::WID("A'"));
    HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
    HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
    HyperEdge * e1 = new HyperEdge(na); exp_graph->AddEdge(e1); e1->AddTail(nb12); e1->AddTail(nb3); na->AddEdge(e1);
    e1->SetScore(1); e1->AddFeature(Dict::WID("feat"), 1);
    HyperEdge * e2 = new HyperEdge(nb12); exp_graph->AddEdge(e2); e2->AddTail(nb1); e2->AddTail(nb2); nb12->AddEdge(e2);
    return exp_graph->CheckEqual(*act_graph);
}

int TestBinarizer::TestBinarizerUnordered() {
    BinarizerDirectional br(BinarizerDirectional::BINARIZE_RIGHT);
    boost::shared_ptr<HyperGraph> act_graph(br.TransformGraph(*unordered_graph_));
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    exp_graph->SetWords(src_);
    HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
    HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
    HyperNode * nb23 = new HyperNode; nb23->SetSpan(make_pair(1,3)); exp_graph->AddNode(nb23); nb23->SetSym(Dict::WID("A'"));
    HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
    HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("."));
    HyperEdge * e1 = new HyperEdge(na); exp_graph->AddEdge(e1); e1->AddTail(nb1); e1->AddTail(nb23); na->AddEdge(e1);
    e1->SetScore(1); e1->AddFeature(Dict::WID("feat"), 1);
    HyperEdge * e2 = new HyperEdge(nb23); exp_graph->AddEdge(e2); e2->AddTail(nb2); e2->AddTail(nb3); nb23->AddEdge(e2);
    HyperEdge * e3 = new HyperEdge(nb1); exp_graph->AddEdge(e3); nb1->AddEdge(e3);
    return exp_graph->CheckEqual(*act_graph);
}

int TestBinarizer::TestBinarizerLeft() {
    BinarizerDirectional br(BinarizerDirectional::BINARIZE_LEFT);
    boost::shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    exp_graph->SetWords(src_);
    HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
    HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("."));
    HyperNode * nb12 = new HyperNode; nb12->SetSpan(make_pair(0,2)); exp_graph->AddNode(nb12); nb12->SetSym(Dict::WID("A'"));
    HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
    HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
    HyperEdge * e1 = new HyperEdge(na); exp_graph->AddEdge(e1); e1->AddTail(nb12); e1->AddTail(nb3); na->AddEdge(e1);
    e1->SetScore(1); e1->AddFeature(Dict::WID("feat"), 1);
    HyperEdge * e2 = new HyperEdge(nb12); exp_graph->AddEdge(e2); e2->AddTail(nb1); e2->AddTail(nb2); nb12->AddEdge(e2);
    return exp_graph->CheckEqual(*act_graph);
}
    
int TestBinarizer::TestBinarizerCKY() {
    BinarizerCKY br;
    boost::shared_ptr<HyperGraph> act_graph(br.TransformGraph(*trinary_graph_));
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    exp_graph->SetWords(src_);
    // The order starts from 0-1, 0-2, 1-2, so first will be 12->1 2, na->1 23, na->12 3
    HyperNode * nb12 = new HyperNode; nb12->SetSpan(make_pair(0,2)); exp_graph->AddNode(nb12); nb12->SetSym(Dict::WID("A'"));
    HyperNode * nb1 = new HyperNode; nb1->SetSpan(make_pair(0,1)); exp_graph->AddNode(nb1);    nb1->SetSym(Dict::WID("B1"));
    HyperNode * nb2 = new HyperNode; nb2->SetSpan(make_pair(1,2)); exp_graph->AddNode(nb2);    nb2->SetSym(Dict::WID("B2"));
    HyperNode * na = new HyperNode; na->SetSpan(make_pair(0,3));  exp_graph->AddNode(na);      na->SetSym(Dict::WID("A" ));
    HyperNode * nb23 = new HyperNode; nb23->SetSpan(make_pair(1,3)); exp_graph->AddNode(nb23); nb23->SetSym(Dict::WID("A'"));
    HyperNode * nb3 = new HyperNode; nb3->SetSpan(make_pair(2,3)); exp_graph->AddNode(nb3);    nb3->SetSym(Dict::WID("."));
    HyperEdge * e12 = new HyperEdge(nb12); exp_graph->AddEdge(e12); e12->AddTail(nb1); e12->AddTail(nb2); nb12->AddEdge(e12);
    HyperEdge * e1r = new HyperEdge(na); exp_graph->AddEdge(e1r); e1r->AddTail(nb1); e1r->AddTail(nb23); na->AddEdge(e1r);
    HyperEdge * e1l = new HyperEdge(na); exp_graph->AddEdge(e1l); e1l->AddTail(nb12); e1l->AddTail(nb3); na->AddEdge(e1l);
    HyperEdge * e23 = new HyperEdge(nb23); exp_graph->AddEdge(e23); e23->AddTail(nb2); e23->AddTail(nb3); nb23->AddEdge(e23);
    e1l->SetScore(1); e1l->AddFeature(Dict::WID("feat"), 1);
    e1r->SetScore(1); e1r->AddFeature(Dict::WID("feat"), 1);
    return exp_graph->CheckEqual(*act_graph);
}

int TestBinarizer::TestDoubleRight() {
    BinarizerDirectional br(BinarizerDirectional::BINARIZE_RIGHT);
    boost::shared_ptr<HyperGraph> act_graph(br.TransformGraph(*double_graph_));
    boost::shared_ptr<HyperGraph> exp_graph(new HyperGraph);
    exp_graph->SetWords(src_);
    // Add the nodes
    HyperNode * na = new HyperNode;   na->SetSpan(make_pair(0,3));   exp_graph->AddNode(na);   na->SetSym(  Dict::WID("A" ));
    HyperNode * nrx = new HyperNode;  nrx->SetSpan(make_pair(0,3));  exp_graph->AddNode(nrx);  nrx->SetSym( Dict::WID("X" ));
    HyperNode * nry = new HyperNode;  nry->SetSpan(make_pair(0,3));  exp_graph->AddNode(nry);  nry->SetSym( Dict::WID("Y" ));
    HyperNode * nb1 = new HyperNode;  nb1->SetSpan(make_pair(0,1));  exp_graph->AddNode(nb1);  nb1->SetSym( Dict::WID("B1"));
    HyperNode * nx23 = new HyperNode; nx23->SetSpan(make_pair(1,3)); exp_graph->AddNode(nx23); nx23->SetSym(Dict::WID("X'"));
    HyperNode * nb2 = new HyperNode;  nb2->SetSpan(make_pair(1,2));  exp_graph->AddNode(nb2);  nb2->SetSym( Dict::WID("B2"));
    HyperNode * nb3 = new HyperNode;  nb3->SetSpan(make_pair(2,3));  exp_graph->AddNode(nb3);  nb3->SetSym( Dict::WID("." ));
    HyperNode * ny23 = new HyperNode; ny23->SetSpan(make_pair(1,3)); exp_graph->AddNode(ny23); ny23->SetSym(Dict::WID("Y'"));
    // Add the edges
    HyperEdge * ex1 = new HyperEdge(na); exp_graph->AddEdge(ex1); ex1->AddTail(nrx); na->AddEdge(ex1);
    ex1->SetScore(1); ex1->AddFeature(Dict::WID("feat"), 1);
    HyperEdge * ey1 = new HyperEdge(na); exp_graph->AddEdge(ey1); ey1->AddTail(nry); na->AddEdge(ey1);
    ey1->SetScore(1); ey1->AddFeature(Dict::WID("feat"), 1);
    HyperEdge * ex2 = new HyperEdge(nrx);  exp_graph->AddEdge(ex2); ex2->AddTail(nb1); ex2->AddTail(nx23); nrx->AddEdge(ex2);
    HyperEdge * ex3 = new HyperEdge(nx23); exp_graph->AddEdge(ex3); ex3->AddTail(nb2); ex3->AddTail(nb3);  nx23->AddEdge(ex3);
    HyperEdge * ey2 = new HyperEdge(nry);  exp_graph->AddEdge(ey2); ey2->AddTail(nb1); ey2->AddTail(ny23); nry->AddEdge(ey2);
    HyperEdge * ey3 = new HyperEdge(ny23); exp_graph->AddEdge(ey3); ey3->AddTail(nb2); ey3->AddTail(nb3);  ny23->AddEdge(ey3);
    return exp_graph->CheckEqual(*act_graph);
}

bool TestBinarizer::RunTest() {
    int done = 0, succeeded = 0;
    done++; cout << "TestBinarizerRight()" << endl; if(TestBinarizerRight()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBinarizerRightRaisePunc()" << endl; if(TestBinarizerRightRaisePunc()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBinarizerUnordered()" << endl; if(TestBinarizerUnordered()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBinarizerLeft()" << endl; if(TestBinarizerLeft()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestBinarizerCKY()" << endl; if(TestBinarizerCKY()) succeeded++; else cout << "FAILED!!!" << endl;
    done++; cout << "TestDoubleRight()" << endl; if(TestDoubleRight()) succeeded++; else cout << "FAILED!!!" << endl;
    cout << "#### TestBinarizer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
    return done == succeeded;
}

} // namespace travatar

