#ifndef EVAL_MEASURE_RIBES_H__
#define EVAL_MEASURE_RIBES_H__

#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

namespace travatar {

class EvalStatsRibes : public EvalStatsAverage {
public:
    EvalStatsRibes(double val = 0.0, double denom = 1.0) : EvalStatsAverage(val,denom) { }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsRibes(vals_[0], vals_[1])); }
    virtual std::string GetIdString() const { return "RIBES"; }
};

class EvalMeasureRibes : public EvalMeasure {

public:


    EvalMeasureRibes(double alpha = 0.25, double beta = 0.10) :
        RIBES_VERSION_("1.02.3"), alpha_(alpha), beta_(beta)
         { }
    EvalMeasureRibes(const std::string & str);

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys) const;

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file);

protected:
    std::string RIBES_VERSION_;
    double alpha_;
    double beta_;

};

}

#endif
