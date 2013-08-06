
#include <travatar/util.h>
#include <travatar/eval-measure-wer.h>
#include <tr1/unordered_map>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace std::tr1;
using namespace travatar;
using namespace boost;

// Calculate Levenshtein Distance
int EvalMeasureWer::EditDistance(const Sentence & ref, const Sentence & sys) const {
    int rs = ref.size()+1, ss = sys.size()+1;
    vector<int> dists(rs*ss);
    for(int i = 0; i < rs; i++)
        dists[i*ss] = i;
    for(int j = 1; j < ss; j++)
        dists[j] = j;
    for(int i = 1; i < rs; i++)
        for(int j = 1; j < ss; j++) {
            dists[i*ss+j] =
                min(
                    min(dists[(i-1)*ss+j]+1, dists[i*ss+j-1]+1),
                    dists[(i-1)*ss+j-1] + (ref[i-1] == sys[j-1] ? 0 : 1));
        }
    return dists[rs*ss-1];
}

// Measure the score of the sys output according to the ref
shared_ptr<EvalStats> EvalMeasureWer::CalculateStats(const Sentence & ref, const Sentence & sys) const {

    return shared_ptr<EvalStats>(new EvalStatsWer(EditDistance(ref,sys), ref.size(), reverse_));

}

// Read in the stats
shared_ptr<EvalStats> EvalMeasureWer::ReadStats(const std::string & line) {
    EvalStatsPtr ret(new EvalStatsWer(0, 0, reverse_));
    ret->ReadStats(line);
    return ret;
}


EvalMeasureWer::EvalMeasureWer(const std::string & config)
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
