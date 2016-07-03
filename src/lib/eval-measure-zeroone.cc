
#include <travatar/global-debug.h>
#include <travatar/eval-measure-zeroone.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <cmath>

using namespace std;
using namespace travatar;
using namespace boost;

// Measure the score of the sys output according to the ref
boost::shared_ptr<EvalStats> EvalMeasureZeroOne::CalculateStats(const Sentence & ref, const Sentence & sys) const {

    return boost::shared_ptr<EvalStats>(new EvalStatsZeroOne((ref == sys ? 1 : 0), 1));

}

// Read in the stats
boost::shared_ptr<EvalStats> EvalMeasureZeroOne::ReadStats(const std::string & line) {
    EvalStatsPtr ret(new EvalStatsZeroOne());
    ret->ReadStats(line);
    return ret;
}

EvalMeasureZeroOne::EvalMeasureZeroOne(const std::string & config) { }
