#ifndef EVAL_MEASURE_WER_H__
#define EVAL_MEASURE_WER_H__

#include <map>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

class EvalStatsWer : public EvalStatsAverage {
public:
    EvalStatsWer(double val, double denom = 1.0, bool reverse = false)
        : EvalStatsAverage(val,denom), reverse_(reverse) { }
    virtual std::string GetIdString() const { return "WER"; }
    virtual double ConvertToScore() const {
        double score = vals_[1] ? vals_[0]/vals_[1] : 0;
        return reverse_ ? 1-score : score;
    }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsWer(vals_[0], vals_[1], reverse_)); }
protected:
    bool reverse_;
};

class EvalMeasureWer : public EvalMeasure {

public:

    EvalMeasureWer(bool reverse = false) : reverse_ (reverse) { }
    EvalMeasureWer(const std::string & str);

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys) const;

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file);

protected:

    int EditDistance(const Sentence & ref, const Sentence & sys) const;
    
    // WER is better when it is lower, so for tuning we want to be able to
    // subtract WER from 1 to get a value that is better when it is higher
    bool reverse_;

};

}

#endif
