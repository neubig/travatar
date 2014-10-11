
#include <travatar/global-debug.h>
#include <travatar/dict.h>
#include <travatar/eval-measure-adv-interp.h>
#include <travatar/math-query.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace travatar;
using namespace boost;

std::string EvalStatsAdvInterp::ConvertToString() const {
    std::ostringstream oss;
    map<WordId, double> var_map;
    for (size_t i=0; i < stats_.size(); ++i) 
        var_map.insert(make_pair(vars_[i],stats_[i]->ConvertToScore()));
    MathQuery mq(query_,var_map);
    mq.Print(oss);
    return oss.str();
}

std::string EvalStatsAdvInterp::GetIdString() const { return "ADV-INTERP"; }
double EvalStatsAdvInterp::ConvertToScore() const {
    map<WordId, double> var_map;
    for(size_t i = 0; i < stats_.size(); ++i) 
        var_map.insert(make_pair(vars_[i],stats_[i]->ConvertToScore()));
    return MathQuery::Evaluate(var_map,query_);
}
// Check if the value is zero
bool EvalStatsAdvInterp::IsZero() {
    BOOST_FOREACH(const EvalStatsPtr & ptr, stats_)
        if(!ptr->IsZero())
            return false;
    return true;
}
EvalStats & EvalStatsAdvInterp::PlusEquals(const EvalStats & rhs) {
    const EvalStatsAdvInterp & rhsi = (const EvalStatsAdvInterp &)rhs;
    if(stats_.size() != rhsi.stats_.size())
        THROW_ERROR("Interpreted eval measure sizes don't match");
    for(int i = 0; i < (int)stats_.size(); i++)
        stats_[i]->PlusEquals(*rhsi.stats_[i]);
    return *this;
}
EvalStats & EvalStatsAdvInterp::TimesEquals(EvalStatsDataType mult) {
    for(int i = 0; i < (int)stats_.size(); i++)
        stats_[i]->TimesEquals(mult);
    return *this;
}
bool EvalStatsAdvInterp::Equals(const EvalStats & rhs) const {
    const EvalStatsAdvInterp & rhsi = (const EvalStatsAdvInterp &)rhs;
    if(stats_.size() != rhsi.stats_.size()) return false;
    for(int i = 0; i < (int)stats_.size(); i++) {
        if(!stats_[i]->Equals(*rhsi.stats_[i]) || vars_[i] != rhsi.vars_[i] || query_[i] != rhsi.query_[i])
            return false;
    }
    return true;
}

EvalStatsPtr EvalStatsAdvInterp::Clone() const { 
    std::vector<EvalStatsPtr> newstats;
    BOOST_FOREACH(const EvalStatsPtr & ptr, stats_)
        newstats.push_back(ptr->Clone());
    return EvalStatsPtr(new EvalStatsAdvInterp(newstats, vars_, query_));
}

// Measure the score of the sys output according to the ref
EvalStatsPtr EvalMeasureAdvInterp::CalculateStats(const Sentence & ref, const Sentence & sys) const {
    // Calculate all the stats independently and add them
    typedef boost::shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateStats(ref,sys));
    return EvalStatsPtr(new EvalStatsAdvInterp(stats, vars_, query_));
}

EvalStatsPtr EvalMeasureAdvInterp::CalculateCachedStats(
            const std::vector<Sentence> & refs, const std::vector<Sentence> & syss, int ref_cache_id, int sys_cache_id) {
    typedef boost::shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateCachedStats(refs,syss,ref_cache_id,sys_cache_id));
    return EvalStatsPtr(new EvalStatsAdvInterp(stats, vars_, query_));
}
EvalStatsPtr EvalMeasureAdvInterp::CalculateCachedStats(
            const std::vector<Sentence> & refs, const CfgDataVector & syss, int ref_cache_id, int sys_cache_id) {
    typedef boost::shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateCachedStats(refs,syss,ref_cache_id,sys_cache_id));
    return EvalStatsPtr(new EvalStatsAdvInterp(stats, vars_, query_));
}

// Read in the stats
EvalStatsPtr EvalMeasureAdvInterp::ReadStats(const std::string & line) {
    std::vector<std::string> cols;
    boost::algorithm::split(cols, line, boost::is_any_of("\t"));
    if(cols.size() != measures_.size())
        THROW_ERROR("Number of columns in input ("<<cols.size()<<") != number of evaluation measures (" << measures_.size() << ")");
    // Load the stats
    typedef boost::shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats(cols.size());
    for(int i = 0; i < (int)cols.size(); i++)
        stats[i] = measures_[i]->ReadStats(cols[i]);
    return EvalStatsPtr(new EvalStatsAdvInterp(stats, vars_, query_));
}

EvalMeasureAdvInterp::EvalMeasureAdvInterp(const std::string & config) {
    vector<string> strs;
    boost::algorithm::split(strs, config, boost::is_any_of("|"));
    if(strs.size() == 0 || strs.size() % 2 != 1)
        THROW_ERROR("Bad configuration in interpreted evaluation measure: " << config);
    for(size_t i = 0; i < strs.size()-1; i += 2) {
        if (strs[i].size() != 1 || strs[i][0] < 'A' || strs[i][0] > 'Z')
            THROW_ERROR("Variable should be within A-Z (inclusive)");
        WordId var = Dict::WID(strs[i]);
        vars_.push_back(var);
        measures_.push_back(boost::shared_ptr<EvalMeasure>(EvalMeasure::CreateMeasureFromString(strs[i+1])));
    }
    query_ = strs[strs.size()-1];
}

std::string EvalStatsAdvInterp::WriteStats() {
    std::ostringstream oss;
    for(int i = 0; i < (int)stats_.size(); i++) {
        if(i) oss << '\t';
        oss << stats_[i]->WriteStats();
    }
    return oss.str();
}
