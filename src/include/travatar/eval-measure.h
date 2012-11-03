#ifndef EVAL_MEASURE_H__
#define EVAL_MEASURE_H__

#include <vector>
#include <travatar/util.h>
#include <travatar/dict.h>

namespace travatar {

class EvalMeasure {

public:

    EvalMeasure() { }

    // Measure the score of the system output according to the reference
    virtual double MeasureScore(
            const std::vector<WordId> & reference,
            const std::vector<WordId> & system) = 0;

    // Measure the loss of the system output according to the reference
    // If the scores are guaranteed to be between 0 and 1, does not need
    // to be overridden
    virtual double MeasureLoss(
            const std::vector<WordId> & reference,
            const std::vector<WordId> & system) {
        return 1 - MeasureScore(reference, system);
    }

protected:

};

}

#endif
