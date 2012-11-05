#ifndef EVAL_MEASURE_RIBES_H__
#define EVAL_MEASURE_RIBES_H__

#include <map>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/eval-measure.h>

namespace travatar {

class EvalMeasureRibes : public EvalMeasure {

public:


    EvalMeasureRibes(double alpha = 0.25, double beta = 0.10) :
        RIBES_VERSION_("1.02.3"), alpha_(alpha), beta_(beta)
         { }

    // Measure the score of the system output according to the reference
    virtual double MeasureScore(
            const Sentence & reference,
            const Sentence & system) const;

    // int GetNgramOrder() const { return ngram_order_; }
    // void SetNgramOrder(int ngram_order) { ngram_order_ = ngram_order; }

protected:
    std::string RIBES_VERSION_;
    double alpha_;
    double beta_;

};

}

#endif
