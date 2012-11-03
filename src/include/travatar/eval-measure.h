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

    // Measure the score of the sys output according to the ref
    virtual double MeasureScore(
            const Sentence & ref,
            const Sentence & sys) const = 0;

    // Measure the loss of the sys output according to the ref
    // If the scores are guaranteed to be between 0 and 1, does not need
    // to be overridden
    virtual double MeasureLoss(
            const Sentence & ref,
            const Sentence & sys) const {
        return 1 - MeasureScore(ref, sys);
    }

    // Measure the loss of the sys output according to multiple refs
    virtual double MeasureLoss(
            const std::vector<Sentence> & refs,
            const Sentence & sys) const {
        double ret = DBL_MAX;
        BOOST_FOREACH(const Sentence & ref, refs)
            ret = std::min(ret, MeasureLoss(ref, sys));
        return ret;
    }

protected:

};

}

#endif
