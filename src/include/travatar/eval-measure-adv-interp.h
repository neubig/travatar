#ifndef EVAL_MEASURE_ADV_INTERP_H__
#define EVAL_MEASURE_ADV_INTERP_H__

// In interpreted version of several evaluation measures
// Specify it as follows
//  interp=0.4|bleu|0.6|ribes
// To do the 0.4/0.6 interperation fo the BLEU and RIBES measures

#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

namespace travatar {

// The interpolated stats
class EvalStatsAdvInterp : public EvalStats {
public:
    EvalStatsAdvInterp(const std::vector<EvalStatsPtr> & stats = std::vector<EvalStatsPtr>(),
            const std::vector<WordId> vars = std::vector<WordId>(), const std::string query = std::string()) :
        stats_(stats), vars_(vars), query_(query) { }
    virtual ~EvalStatsAdvInterp() { }

    virtual std::string ConvertToString() const;
    virtual std::string GetIdString() const;
    virtual Real ConvertToScore() const;
    // Check if the value is zero
    virtual bool IsZero();
    virtual EvalStats & PlusEquals(const EvalStats & rhs);
    virtual EvalStats & TimesEquals(EvalStatsDataType mult);
    virtual bool Equals(const EvalStats & rhs) const;

    EvalStatsPtr Clone() const;

    virtual std::string WriteStats();

protected:
    std::vector<EvalStatsPtr> stats_;
    std::vector<WordId> vars_; 
    std::string query_;
};

// The interpolated evaluation measure
class EvalMeasureAdvInterp : public EvalMeasure {

public:


    EvalMeasureAdvInterp(const std::vector<boost::shared_ptr<EvalMeasure> > & measures, const std::vector<WordId> & vars, std::string query) 
        : measures_(measures), vars_(vars), query_(query) { }
    EvalMeasureAdvInterp(const std::string & str);
    virtual ~EvalMeasureAdvInterp() { }

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys) const;
    virtual EvalStatsPtr CalculateCachedStats(
                const std::vector<Sentence> & ref,
                const std::vector<Sentence> & syss,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX);
    virtual EvalStatsPtr CalculateCachedStats(
                const std::vector<Sentence> & ref,
                const CfgDataVector & syss,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX);

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file);

protected:
    std::vector<boost::shared_ptr<EvalMeasure> > measures_;
    std::vector<WordId> vars_;
    std::string query_;
};

}

#endif
