#ifndef EVAL_MEASURE_ZEROONE_H__
#define EVAL_MEASURE_ZEROONE_H__

#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <travatar/real.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

namespace travatar {

class EvalStatsZeroOne : public EvalStatsAverage {
public:
    EvalStatsZeroOne(Real val = 0.0, Real denom = 1.0) : EvalStatsAverage(val,denom) { }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsZeroOne(vals_[0], vals_[1])); }
    virtual std::string GetIdString() const { return "ZEROONE"; }
};

class EvalMeasureZeroOne : public EvalMeasure {

public:

    EvalMeasureZeroOne() { }
    EvalMeasureZeroOne(const std::string & str);

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys) const;

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file);

};

}

#endif
