
#include <travatar/util.h>
#include <travatar/eval-measure-ter.h>
#include <tr1/unordered_map>
#include <tercalc.h>
#include <terAlignment.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace std::tr1;
using namespace travatar;
using namespace boost;
using namespace TERCpp;

// Measure the score of the sys output according to the ref
shared_ptr<EvalStats> EvalMeasureTer::CalculateStats(const Sentence & ref, const Sentence & sys, int ref_cache_id, int sys_cache_id) {

  terCalc * evaluation = new terCalc;
  terAlignment result = evaluation->TER ( ref, sys );
  delete evaluation;

  ostringstream stats;
  // multiplication by 100 in order to keep the average precision
  // in the TER calculation.
  return shared_ptr<EvalStats>(new EvalStatsTer(result.numEdits, ref.size(), reverse_));
  // stats << result.numEdits*100.0 << " " << result.averageWords*100.0 << " " << result.scoreAv()*100.0 << " " ;

}

// Read in the stats
shared_ptr<EvalStats> EvalMeasureTer::ReadStats(const std::string & line) {
    EvalStatsPtr ret(new EvalStatsTer(0, 0, reverse_));
    ret->ReadStats(line);
    return ret;
}


EvalMeasureTer::EvalMeasureTer(const std::string & config)
                        : reverse_(false) {
    if(config.length() == 0) return;
    BOOST_FOREACH(const EvalMeasure::StringPair & strs, EvalMeasure::ParseConfig(config)) {
        if(strs.first == "reverse") {
            if(strs.second == "true")
                reverse_ = true;
            else if(strs.second == "false")
                reverse_ = false;
            else
                THROW_ERROR("Bad reverse value: " << strs.second);
        } else {
            THROW_ERROR("Bad configuration string: " << config);
        }
    }
}
