
#include <travatar/util.h>
#include <travatar/eval-measure-interp.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <tr1/unordered_map>

using namespace std;
using namespace std::tr1;
using namespace travatar;
using namespace boost;

// Measure the score of the sys output according to the ref
EvalStatsPtr EvalMeasureInterp::CalculateStats(const Sentence & ref, const Sentence & sys) const {

    // Calculate all the stats independently and add them
    typedef shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateStats(ref,sys));
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
