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
    // Get the ID of this stats
    virtual std::string GetIdString() const = 0;
    // Check if the value is zero
    virtual bool IsZero() {
        BOOST_FOREACH(const EvalStatsDataType & val, vals_)
            if(val != 0)
                return false;
        return true;
    }
    // Utility functions
    virtual std::string ConvertToString() const {
        std::ostringstream oss;
        oss << GetIdString() << " = " << ConvertToScore();
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
    virtual bool Equals(const EvalStats & rhs) const {
        if(vals_.size() != rhs.vals_.size()) return false;
        for(int i = 0; i < (int)vals_.size(); i++) {
            if(fabs(vals_[i]-rhs.vals_[i]) > 1e-6)
                return false;
        }
        return true;
    }
    const std::vector<EvalStatsDataType> & GetVals() const { return vals_; }
    virtual void ReadStats(const std::string & str) {
        vals_.resize(0);
        EvalStatsDataType val;
        std::istringstream iss(str);
        while(iss >> val)
            vals_.push_back(val);
    }
    virtual std::string WriteStats() {
        std::ostringstream oss;
        for(int i = 0; i < (int)vals_.size(); i++) {
            if(i) oss << ' ';
            oss << vals_[i];
        }
        return oss.str();
    }
protected:
    std::vector<EvalStatsDataType> vals_;
};

inline bool operator==(const EvalStats & lhs, const EvalStats & rhs) {
    return lhs.Equals(rhs);
}
inline bool operator==(const EvalStatsPtr & lhs, const EvalStatsPtr & rhs) {
    return *lhs == *rhs;
}
inline std::ostream &operator<<( std::ostream &out, const EvalStats &L ) {
    out << L.ConvertToString();
    return out;
}

// Simple sentence-averaged stats
class EvalStatsAverage : public EvalStats {
public:
    EvalStatsAverage(double val = 0.0, double denom = 1.0) {
        vals_.resize(2);
        vals_[0] = val;
        vals_[1] = denom;
    }
    double ConvertToScore() const { return vals_[1] ? vals_[0]/vals_[1] : 0; }
    EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsAverage(vals_[0], vals_[1])); }
    virtual std::string GetIdString() const { return "AVG"; }
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

    typedef std::pair<std::string, std::string> StringPair;

    // Create with default settings
    EvalMeasure() { }
    // Create with configuration
    EvalMeasure(const std::string & config) { }
    virtual ~EvalMeasure() { }

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr CalculateStats(
                const Sentence & ref,
                const Sentence & sys) const = 0;

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr CalculateCachedStats(
                const Sentence & ref,
                const Sentence & sys,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX) {
        return CalculateStats(ref,sys);
    }

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file) = 0;

    // Parse a pair of strings
    static std::vector<StringPair> ParseConfig(const std::string & config);

    // Create measure from string
    static EvalMeasure * CreateMeasureFromString(const std::string & eval);
    
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
