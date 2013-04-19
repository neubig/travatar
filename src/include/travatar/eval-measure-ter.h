#ifndef EVAL_MEASURE_TER_H__
#define EVAL_MEASURE_TER_H__

#include <map>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

class EvalStatsTer : public EvalStatsAverage {
public:
    EvalStatsTer(double val, double denom = 1.0, bool reverse = false)
        : EvalStatsAverage(val,denom), reverse_(reverse) { }
    virtual std::string ConvertToString() const {
        std::ostringstream oss;
        oss << "TER = " << ConvertToScore();
        return oss.str();
    }
    virtual double ConvertToScore() const {
        double score = vals_[1] ? vals_[0]/vals_[1] : 0;
        return reverse_ ? 1-score : score;
    }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsTer(vals_[0], vals_[1])); }
protected:
    bool reverse_;
};

class EvalMeasureTer : public EvalMeasure {

public:

    EvalMeasureTer(bool reverse = false) : reverse_ (reverse) { }
    EvalMeasureTer(const std::string & str);

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX);

protected:
    
    // TER is better when it is lower, so for tuning we want to be able to
    // subtract TER from 1 to get a value that is better when it is higher
    bool reverse_;

};

}

#endif
