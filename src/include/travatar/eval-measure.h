#ifndef EVAL_MEASURE_H__
#define EVAL_MEASURE_H__

#include <cfloat>
#include <vector>
#include <travatar/util.h>
#include <travatar/dict.h>

namespace travatar {

class EvalMeasure {

public:

    EvalMeasure() { }

    // Measure the score of the system output according to the reference
    virtual double MeasureScore(
            const Sentence & reference,
            const Sentence & system) const = 0;

    // Measure the loss of the system output according to the reference
    // If the scores are guaranteed to be between 0 and 1, does not need
    // to be overridden
    virtual double MeasureLoss(
            const Sentence & reference,
            const Sentence & system) const {
        return 1 - MeasureScore(reference, system);
    }

    // Measure the loss of the system output according to multiple references
    virtual double MeasureLoss(
            const std::vector<Sentence> & references,
            const Sentence & system) const {
        double ret = DBL_MAX;
        BOOST_FOREACH(const Sentence & reference, references)
            ret = std::min(ret, MeasureLoss(reference, system));
        return ret;
    }

protected:

};

}

#endif
