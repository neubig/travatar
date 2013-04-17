#ifndef EVAL_MEASURE_INTERP_H__
#define EVAL_MEASURE_INTERP_H__

// In interpreted version of several evaluation measures
// Specify it as follows
//  interp=0.4|bleu|0.6|ribes
// To do the 0.4/0.6 interperation fo the BLEU and RIBES measures

#include <map>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>

namespace travatar {

// The interpolated stats
class EvalStatsInterp : public EvalStats {
public:
    EvalStatsInterp(const std::vector<EvalStatsPtr> & stats, const std::vector<double> & coeffs) :
        stats_(stats), coeffs_(coeffs) { }

    virtual std::string ConvertToString() const {
        std::ostringstream oss;
        oss << "INTERP = " << ConvertToScore() << " (";
        for(int i = 0; i < (int)stats_.size(); i++) {
            if(i != 0) oss << " + ";
            oss << stats_[i]->ConvertToScore() << '*' << coeffs_[i];
        }
        oss << ")/Z";
        return oss.str();
    }
    virtual double ConvertToScore() const {
        double num = 0, denom = 0;
        for(int i = 0; i < (int)stats_.size(); i++) {
            num   += coeffs_[i] * stats_[i]->ConvertToScore();
            denom += coeffs_[i];
        }

        return num/denom;
    }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsInterp(stats_, coeffs_)); }
protected:
    std::vector<EvalStatsPtr> stats_;
    std::vector<double> coeffs_;
};

// The interpolated evaluation measure
class EvalMeasureInterp : public EvalMeasure {

public:


    EvalMeasureInterp(const std::vector<boost::shared_ptr<EvalMeasure> > & measures, const std::vector<double> & coeffs) 
        : measures_(measures), coeffs_(coeffs) { }
    EvalMeasureInterp(const std::string & str);

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX);

    // int GetNgramOrder() const { return ngram_order_; }
    // void SetNgramOrder(int ngram_order) { ngram_order_ = ngram_order; }

protected:
    std::vector<boost::shared_ptr<EvalMeasure> > measures_;
    std::vector<double> coeffs_;

};

}

#endif
