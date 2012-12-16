#ifndef EVAL_MEASURE_H__
#define EVAL_MEASURE_H__

#include <cfloat>
#include <vector>
#include <climits>
#include <travatar/sentence.h>

namespace travatar {

class HyperGraph;

class EvalMeasure {

public:

    EvalMeasure() { }

    // Measure the score of the sys output according to the ref.
    // ref_cache_id and sys_cache_id can be used to cache the
    // sufficient statistics for more efficient processing, or left
    // at the default INT_MAX if caching should not be performed
    virtual double MeasureScore(
            const Sentence & ref,
            const Sentence & sys,
            int ref_cache_id = INT_MAX,
            int sys_cache_id = INT_MAX) = 0;

    // Measure the loss of the sys output according to the ref
    // If the scores are guaranteed to be between 0 and 1, does not need
    // to be overridden
    virtual double MeasureLoss(
            const Sentence & ref,
            const Sentence & sys,
            int ref_cache_id = INT_MAX,
            int sys_cache_id = INT_MAX) {
        return 1 - MeasureScore(ref, sys);
    }

    // Measure the loss of the sys output according to multiple refs
    virtual double MeasureLoss(
            const std::vector<Sentence> & refs,
            const Sentence & sys) {
        double ret = DBL_MAX;
        for(int i = 0; i < (int)refs.size(); i++)
            ret = std::min(ret, MeasureLoss(refs[i], sys));
        return ret;
    }

    // Find the oracle sentence for this evaluatio measure
    virtual Sentence CalculateOracle(const HyperGraph & graph, const Sentence & ref);

    // Clear the cache
    virtual void ClearCache() { }

protected:

};

}

#endif
