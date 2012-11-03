#ifndef EVAL_MEASURE_BLEU_H__
#define EVAL_MEASURE_BLEU_H__

#include <travatar/eval-measure.h>

namespace travatar {

class EvalMeasureBleu : public EvalMeasure {

public:

    EvalMeasureBleu() { }

    // Measure the score of the system output according to the reference
    virtual double MeasureScore(
            const std::vector<WordId> & reference,
            const std::vector<WordId> & system) {
        THROW_ERROR("EvalMeasureBleu::MeasureScore not implemented");
    }

    double GetSmoothVal() const { return smooth_val_; }
    void SetSmoothVal(double smooth_val) { smooth_val_ = smooth_val; }

protected:
    double smooth_val_;

};

}

#endif
