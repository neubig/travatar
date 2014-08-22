#ifndef TRAVATAR_GRADIENT_H__
#define TRAVATAR_GRADIENT_H__

#include <travatar/sparse-map.h>
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

class TuningExample;

class Gradient {
public:
    Gradient();
    virtual ~Gradient() { }

    // Initialize the statistics necessary for gradient calculation
    // This should calculate all the statistics that are not influenced by the
    // weight vector itself
    virtual void Init(const std::vector<boost::shared_ptr<TuningExample> > & examps);

    // Calculate the gradient for particular weights using either the dense or sparse
    // representation
    virtual double CalcSparseGradient(const SparseMap & weights, SparseMap & d_xeval_dw) const;
    virtual double CalcGradient(size_t n, const double * x, double * g) const = 0;

    void DensifyWeights(const SparseMap & sparse, std::vector<double> & dense);
    void SparsifyWeights(const std::vector<double> & dense, SparseMap & sparse);

    // Accessors
    void SetL2Coefficient(double l2_coeff) { l2_coeff_ = l2_coeff; }
    void SetAutoScale(bool auto_scale) { auto_scale_ = auto_scale; }
    void SetMult(double mult) { mult_ = mult; }
    double GetMult() const { return mult_; }

protected:

    std::vector<int> dense2sparse_;
    SparseIntMap sparse2dense_;
    std::vector<std::vector<EvalStatsPtr> > all_stats_;    
    std::vector<std::vector<std::vector<std::pair<WordId,double> > > > all_feats_;
    const std::vector<boost::shared_ptr<TuningExample> > * examps_ptr_;
    bool auto_scale_;
    WordId dense_scale_id_;
    WordId scale_id_;
    double l2_coeff_;
    double mult_;
    

};

}

#endif
