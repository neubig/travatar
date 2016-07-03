#include <travatar/eval-measure-loader.h>

#include <travatar/eval-measure.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/eval-measure-nist.h>
#include <travatar/eval-measure-ter.h>
#include <travatar/eval-measure-wer.h>
#include <travatar/eval-measure-interp.h>
#include <travatar/eval-measure-adv-interp.h>
#include <travatar/eval-measure-zeroone.h>

#include <travatar/global-debug.h>

using namespace std;
using namespace travatar;

namespace travatar {

EvalMeasure * EvalMeasureLoader::CreateMeasureFromString(const string & str) {
    // Get the eval, config substr
    string eval, config;
    size_t eq = str.find(':');
    if(eq == string::npos) { eval = str; }
    else { eval = str.substr(0,eq); config = str.substr(eq+1); }
    // Create the actual measure
    if(eval == "bleu") 
        return new EvalMeasureBleu(config);
    else if(eval == "ribes")
        return new EvalMeasureRibes(config);
    else if(eval == "nist")
        return new EvalMeasureNist(config);
    else if(eval == "ter")
        return new EvalMeasureTer(config);
    else if(eval == "wer")
        return new EvalMeasureWer(config);
    else if(eval == "interp")
        return new EvalMeasureInterp(config);
    else if(eval == "ainterp")
        return new EvalMeasureAdvInterp(config);
    else if(eval == "zeroone")
        return new EvalMeasureZeroOne(config);
    else
        THROW_ERROR("Unknown evaluation measure: " << eval);
    return NULL;
}

} // namespace travatar

