#include <travatar/gradient.h>
#include <travatar/global-debug.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/tuning-example.h>
#include <travatar/weights.h>
#include <travatar/dict.h>
#include <boost/foreach.hpp>
#include <cmath>
#include <set>

using namespace std;
using namespace boost;
using namespace travatar;

Gradient::Gradient() : examps_ptr_(NULL), auto_scale_(false), dense_scale_id_(-1), scale_id_(Dict::WID("__SCALE__")), l2_coeff_(0.0), mult_(1.0) { }

void Gradient::DensifyWeights(const SparseMap & sparse, std::vector<double> & dense) {
    dense.resize(dense2sparse_.size());
    for(int i = 0; i < (int)dense2sparse_.size(); i++) {
        SparseMap::const_iterator it = sparse.find(dense2sparse_[i]);
        dense[i] = (it == sparse.end() ? 0.0 : it->second);
    }
    if(dense_scale_id_ != -1 && dense[dense_scale_id_] == 0.0)
        dense[dense_scale_id_] = 1.0;
}

void Gradient::SparsifyWeights(const std::vector<double> & dense, SparseMap & sparse) {
    sparse = SparseMap();
    for(int i = 0; i < (int)dense2sparse_.size(); i++)
        if(dense[i])
            sparse[dense2sparse_[i]] = dense[i];
}

double Gradient::CalcSparseGradient(const SparseMap & kv, SparseMap & d_xeval_dw) const {
    size_t n = dense2sparse_.size();
    vector<double> x(n, 0.0);
    BOOST_FOREACH(const SparseMap::value_type val, kv) {
        SparseIntMap::const_iterator it = sparse2dense_.find(val.first);
        if(it == sparse2dense_.end())
            THROW_ERROR("Attempting to transform value not in sparse2dense map: " << val.first << " (" << Dict::WSym(val.first) << ")" << endl);
        x[it->second] = val.second;
    }
    vector<double> g(n, 0.0);
    double ret = CalcGradient(n, &x[0], &g[0]);
    for(size_t i = 0; i < n; i++)
        if(g[i] != 0.0)
            d_xeval_dw[dense2sparse_[i]] = g[i];
    return ret;
}

void Gradient::Init(const SparseMap & init_weights, const std::vector<boost::shared_ptr<TuningExample> > & examps) {
    // If we are using a different example set, re-initialize
    if(examps_ptr_ != &examps) {
        set<WordId> potential;
        BOOST_FOREACH(const boost::shared_ptr<TuningExample> & examp, examps)
            examp->CountWeights(potential);
        BOOST_FOREACH(const SparseMap::value_type & val, init_weights)
            potential.insert(val.first);
        potential.insert(scale_id_);
        BOOST_FOREACH(WordId val, potential) {
            sparse2dense_[val] = dense2sparse_.size();
            dense2sparse_.push_back(val);
        }
        // Build the stats
        all_stats_.clear(); all_stats_.resize(examps.size());
        all_feats_.clear(); all_feats_.resize(examps.size());
        for(int i = 0; i < (int)examps.size(); i++) {
            const vector<ExamplePair> & nbest = examps[i]->CalculateNbest(Weights());
            int K = nbest.size();
            all_stats_[i].resize(K);
            all_feats_[i].resize(K);
            for(int k = 0; k < K; k++) {
                all_stats_[i][k] = nbest[k].second;
                BOOST_FOREACH(SparsePair val, nbest[k].first.GetImpl())
                    all_feats_[i][k].push_back(make_pair(sparse2dense_[val.first], val.second));
            }
        }
        dense_scale_id_ = sparse2dense_[scale_id_];
        examps_ptr_ = &examps;
    }
}
