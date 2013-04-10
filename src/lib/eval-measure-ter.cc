
#include <travatar/util.h>
#include <travatar/eval-measure-ter.h>
#include <tr1/unordered_map>
#include <tercalc.h>
#include <terAlignment.h>

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
