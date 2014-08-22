#ifndef TRAVATAR_GRADIENT_XEVAL_H__
#define TRAVATAR_GRADIENT_XEVAL_H__

#include <travatar/gradient.h>
#include <travatar/eval-measure.h>

namespace travatar {

class GradientXeval : public Gradient {
public:

    GradientXeval();

    // Calculate the gradient
    virtual double CalcGradient(size_t n, const double * x, double * g) const;

    // Calculate the gradient for averaged measures based on expectations, probabilities
    void CalcAvgGradient(
            const std::vector<std::vector<double> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const double * x, double * g) const;
    
    // Calculate the gradient for BLEU based on expectations, probabilities
    void CalcBleuGradient(
            const std::vector<std::vector<double> > & p_i_k,
            const EvalStatsPtr & stats, 
            size_t n, const double * x, double * g) const;

    // Set the entropy coefficient
    void SetEntCoefficient(double ent_coeff) { ent_coeff_ = ent_coeff; }

protected:

    double ent_coeff_;
    

};

}

#endif
