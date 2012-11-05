#ifndef TUNER_GREEDY_MERT_H__
#define TUNER_GREEDY_MERT_H__

namespace travatar {



// Does MERT in a greedy fashion, at each point testing the value that
// could potentially raise the evaluation by the largest amount. At the
// moment this only works for sentence-decomposable evaluation measures.
class TunerGreedyMert {

public:

    TunerGreedyMert();

    SparseMap Tune(const vector<vector<pair<SparseMap, double> > > & examps,
                   const SparseMap & init = SparseMap());

protected:

};

}

#endif
