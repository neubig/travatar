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
    EvalStatsInterp(const std::vector<EvalStatsPtr> & stats = std::vector<EvalStatsPtr>(),
                    const std::vector<double> & coeffs = std::vector<double>()) :
        stats_(stats), coeffs_(coeffs) { }
    virtual ~EvalStatsInterp() { }

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
    virtual std::string GetIdString() const { return "INTERP"; }
    virtual double ConvertToScore() const {
        double num = 0, denom = 0;
        for(int i = 0; i < (int)stats_.size(); i++) {
            num   += coeffs_[i] * stats_[i]->ConvertToScore();
            denom += coeffs_[i];
        }

        return num/denom;
    }
    // Check if the value is zero
    virtual bool IsZero() {
        BOOST_FOREACH(const EvalStatsPtr & ptr, stats_)
            if(!ptr->IsZero())
                return false;
        return true;
    }
    virtual EvalStats & PlusEquals(const EvalStats & rhs) {
        const EvalStatsInterp & rhsi = (const EvalStatsInterp &)rhs;
        if(stats_.size() != rhsi.stats_.size())
            THROW_ERROR("Interpreted eval measure sizes don't match");
        for(int i = 0; i < (int)stats_.size(); i++)
            stats_[i]->PlusEquals(*rhsi.stats_[i]);
        return *this;
    }
    virtual EvalStats & TimesEquals(EvalStatsDataType mult) {
        for(int i = 0; i < (int)stats_.size(); i++)
            stats_[i]->TimesEquals(mult);
        return *this;
    }
    virtual bool Equals(const EvalStats & rhs) const {
        const EvalStatsInterp & rhsi = (const EvalStatsInterp &)rhs;
        if(stats_.size() != rhsi.stats_.size()) return false;
        for(int i = 0; i < (int)stats_.size(); i++) {
            if(!stats_[i]->Equals(*rhsi.stats_[i]) || coeffs_[i] != rhsi.coeffs_[i])
                return false;
        }
        return true;
    }

    EvalStatsPtr Clone() const { 
        std::vector<EvalStatsPtr> newstats;
        BOOST_FOREACH(const EvalStatsPtr & ptr, stats_)
            newstats.push_back(ptr->Clone());
        return EvalStatsPtr(new EvalStatsInterp(newstats, coeffs_));
    }

    virtual std::string WriteStats();

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
    virtual ~EvalMeasureInterp() { }

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX);

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file);

protected:
    std::vector<boost::shared_ptr<EvalMeasure> > measures_;
    std::vector<double> coeffs_;

};

}

#endif
