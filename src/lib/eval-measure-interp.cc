
#include <travatar/global-debug.h>
#include <travatar/eval-measure-interp.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <tr1/unordered_map>

using namespace std;
using namespace std::tr1;
using namespace travatar;
using namespace boost;

std::string EvalStatsInterp::ConvertToString() const {
    std::ostringstream oss;
    oss << "INTERP = " << ConvertToScore() << " (";
    for(int i = 0; i < (int)stats_.size(); i++) {
        if(i != 0) oss << " + ";
        oss << stats_[i]->ConvertToScore() << '*' << coeffs_[i];
    }
    oss << ")/Z";
    return oss.str();
}
std::string EvalStatsInterp::GetIdString() const { return "INTERP"; }
double EvalStatsInterp::ConvertToScore() const {
    double num = 0, denom = 0;
    for(int i = 0; i < (int)stats_.size(); i++) {
        num   += coeffs_[i] * stats_[i]->ConvertToScore();
        denom += coeffs_[i];
    }

    return num/denom;
}
// Check if the value is zero
bool EvalStatsInterp::IsZero() {
    BOOST_FOREACH(const EvalStatsPtr & ptr, stats_)
        if(!ptr->IsZero())
            return false;
    return true;
}
EvalStats & EvalStatsInterp::PlusEquals(const EvalStats & rhs) {
    const EvalStatsInterp & rhsi = (const EvalStatsInterp &)rhs;
    if(stats_.size() != rhsi.stats_.size())
        THROW_ERROR("Interpreted eval measure sizes don't match");
    for(int i = 0; i < (int)stats_.size(); i++)
        stats_[i]->PlusEquals(*rhsi.stats_[i]);
    return *this;
}
EvalStats & EvalStatsInterp::TimesEquals(EvalStatsDataType mult) {
    for(int i = 0; i < (int)stats_.size(); i++)
        stats_[i]->TimesEquals(mult);
    return *this;
}
bool EvalStatsInterp::Equals(const EvalStats & rhs) const {
    const EvalStatsInterp & rhsi = (const EvalStatsInterp &)rhs;
    if(stats_.size() != rhsi.stats_.size()) return false;
    for(int i = 0; i < (int)stats_.size(); i++) {
        if(!stats_[i]->Equals(*rhsi.stats_[i]) || coeffs_[i] != rhsi.coeffs_[i])
            return false;
    }
    return true;
}

EvalStatsPtr EvalStatsInterp::Clone() const { 
    std::vector<EvalStatsPtr> newstats;
    BOOST_FOREACH(const EvalStatsPtr & ptr, stats_)
        newstats.push_back(ptr->Clone());
    return EvalStatsPtr(new EvalStatsInterp(newstats, coeffs_));
}

// Measure the score of the sys output according to the ref
EvalStatsPtr EvalMeasureInterp::CalculateStats(const Sentence & ref, const Sentence & sys) const {
    // Calculate all the stats independently and add them
    typedef shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateStats(ref,sys));
    return EvalStatsPtr(new EvalStatsInterp(stats, coeffs_));
}

EvalStatsPtr EvalMeasureInterp::CalculateCachedStats(
            const Sentence & ref, const std::vector<Sentence> & syss, int ref_cache_id, int sys_cache_id) {
    typedef shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateCachedStats(ref,syss,ref_cache_id,sys_cache_id));
    return EvalStatsPtr(new EvalStatsInterp(stats, coeffs_));
}
EvalStatsPtr EvalMeasureInterp::CalculateCachedStats(
            const Sentence & ref, const CfgDataVector & syss, int ref_cache_id, int sys_cache_id) {
    typedef shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateCachedStats(ref,syss,ref_cache_id,sys_cache_id));
    return EvalStatsPtr(new EvalStatsInterp(stats, coeffs_));
}

// Read in the stats
EvalStatsPtr EvalMeasureInterp::ReadStats(const std::string & line) {
    std::vector<std::string> cols;
    boost::algorithm::split(cols, line, boost::is_any_of("\t"));
    if(cols.size() != measures_.size())
        THROW_ERROR("Number of columns in input ("<<cols.size()<<") != number of evaluation measures (" << measures_.size() << ")");
    // Load the stats
    typedef shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats(cols.size());
    for(int i = 0; i < (int)cols.size(); i++)
        stats[i] = measures_[i]->ReadStats(cols[i]);
    return EvalStatsPtr(new EvalStatsInterp(stats, coeffs_));
}

EvalMeasureInterp::EvalMeasureInterp(const std::string & config) {
    vector<string> strs;
    boost::algorithm::split(strs, config, boost::is_any_of("|"));
    if(strs.size() == 0 || strs.size() % 2 != 0)
        THROW_ERROR("Bad configuration in interpreted evaluation measure: " << config);
    for(int i = 0; i < (int)strs.size(); i += 2) {
        coeffs_.push_back(boost::lexical_cast<double>(strs[i]));
        measures_.push_back(boost::shared_ptr<EvalMeasure>(EvalMeasure::CreateMeasureFromString(strs[i+1])));
    }
}

std::string EvalStatsInterp::WriteStats() {
    std::ostringstream oss;
    for(int i = 0; i < (int)stats_.size(); i++) {
        if(i) oss << '\t';
        oss << stats_[i]->WriteStats();
    }
    return oss.str();
}
