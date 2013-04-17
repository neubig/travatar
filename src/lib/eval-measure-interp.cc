
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
shared_ptr<EvalStats> EvalMeasureInterp::CalculateStats(const Sentence & ref, const Sentence & sys, int ref_cache_id, int sys_cache_id) {

    // Calculate all the stats independently and add them
    typedef shared_ptr<EvalMeasure> EvalMeasPtr;
    vector<EvalStatsPtr> stats;
    BOOST_FOREACH(const EvalMeasPtr & meas, measures_)
        stats.push_back(meas->CalculateStats(ref,sys,ref_cache_id,sys_cache_id));
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
