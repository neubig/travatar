#ifndef EVAL_MEASURE_H__
#define EVAL_MEASURE_H__

#include <cfloat>
#include <vector>
#include <climits>
#include <string>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <travatar/sentence.h>
#include <travatar/util.h>

namespace travatar {

class HyperGraph;

// An interface for holding stats for a particular evaluation measure
class EvalStats;
typedef double EvalStatsDataType;
typedef boost::shared_ptr<EvalStats> EvalStatsPtr;
class EvalStats {
public:
    EvalStats() { }
    EvalStats(const std::vector<EvalStatsDataType> & vals) : vals_(vals) { }
    virtual ~EvalStats() { }
    // ConvertToScore takes the stats and converts them into a score
    virtual double ConvertToScore() const = 0;
    // Clone is a utility function that basically calls the constructor of
    // child classes, as this functionality is not included in C++
    virtual EvalStatsPtr Clone() const = 0;
    // Check if the value is zero
    virtual bool IsZero() {
        BOOST_FOREACH(const EvalStatsDataType & val, vals_)
            if(val != 0)
                return false;
        return true;
    }
    // Utility functions
    virtual std::string ConvertToString() {
        std::ostringstream oss;
        oss << ConvertToScore();
        return oss.str();
    }
    virtual EvalStats & PlusEquals(const EvalStats & rhs) {
        if(vals_.size() == 0) {
            vals_ = rhs.vals_;
        } else if (rhs.vals_.size() != 0) {
            if(rhs.vals_.size() != vals_.size())
                THROW_ERROR("Mismatched in EvalStats::PlusEquals");
            for(int i = 0; i < (int)rhs.vals_.size(); i++)
                vals_[i] += rhs.vals_[i];
        }
        return *this;
    }
    virtual EvalStats & TimesEquals(EvalStatsDataType mult) {
        BOOST_FOREACH(EvalStatsDataType & val, vals_)
            val *= mult;
        return *this;
    }
    virtual EvalStatsPtr Plus(const EvalStats & rhs) {
        EvalStatsPtr ret(this->Clone());
        ret->PlusEquals(rhs);
        return ret;
    }
    virtual EvalStatsPtr Times(EvalStatsDataType mult) {
        EvalStatsPtr ret(this->Clone());
        ret->TimesEquals(mult);
        return ret;
    }
protected:
    std::vector<EvalStatsDataType> vals_;
};

// Simple sentence-averaged stats
class EvalStatsAverage : public EvalStats {
public:
    EvalStatsAverage(double val, double denom = 1.0) {
        vals_.resize(2);
        vals_[0] = val;
        vals_[1] = denom;
    }
    double ConvertToScore() const { return vals_[0]/vals_[1]; }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsAverage(vals_[0], vals_[1])); }
    // Getters
    double GetVal() const { return vals_[0]; }
    int GetDenom() const { return vals_[1]; }
private:
};

// An interface for an evaluation measure. All evaluation measures
// must implement CalculateStats(), which returns an EvalStats object
// for measuring the distance between two sentences
class EvalMeasure {

public:

    EvalMeasure() { }

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr CalculateStats(
                const Sentence & ref,
                const Sentence & sys,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX) = 0;

    // // Measure the score of the sys output according to the ref.
    // // ref_cache_id and sys_cache_id can be used to cache the
    // // sufficient statistics for more efficient processing, or left
    // // at the default INT_MAX if caching should not be performed
    // virtual double MeasureScore(
    //         const Sentence & ref,
    //         const Sentence & sys,
    //         int ref_cache_id = INT_MAX,
    //         int sys_cache_id = INT_MAX) = 0;
    //
    // // Measure the loss of the sys output according to the ref
    // // If the scores are guaranteed to be between 0 and 1, does not need
    // // to be overridden
    // virtual double MeasureLoss(
    //         const Sentence & ref,
    //         const Sentence & sys,
    //         int ref_cache_id = INT_MAX,
    //         int sys_cache_id = INT_MAX) {
    //     return 1 - MeasureScore(ref, sys);
    // }
    // 
    // // Measure the loss of the sys output according to multiple refs
    // virtual double MeasureLoss(
    //         const std::vector<Sentence> & refs,
    //         const Sentence & sys) {
    //     double ret = DBL_MAX;
    //     for(int i = 0; i < (int)refs.size(); i++)
    //         ret = std::min(ret, MeasureLoss(refs[i], sys));
    //     return ret;
    // }
    
    // Find the oracle sentence for this evaluation measure
    // TODO: This is totally a hack, doing very ugly things like writing a file to
    //       a specific place on disk, not accounting for sentence brevity, etc.
    //       This needs to be fixed.
    virtual Sentence CalculateOracle(const HyperGraph & graph, const Sentence & ref);

    // Clear the cache
    virtual void ClearCache() { }

protected:

};

}

#endif
