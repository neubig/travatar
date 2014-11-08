
#include <travatar/weights.h>
#include <travatar/hyper-graph.h>
#include <travatar/eval-measure.h>
#include <travatar/global-debug.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace travatar;

void Weights::AdjustNbest(
        const std::vector<std::pair<double,double> > & scores,
        const std::vector<SparseVector*> & features) {
    THROW_ERROR("Standard weights cannot be adjusted");
}

void Weights::Update (
    const SparseVector & oracle, double oracle_score, double oracle_eval,
    const SparseVector & system, double system_score, double system_eval
) {
    THROW_ERROR("Standard weights cannot be updated");
}

// Adjust based on a single one-best list
void Weights::Adjust(const Sentence & src,
                     const std::vector<Sentence> & refs,
                     const EvalMeasure & eval,
                     const NbestList & nbest) {
    std::vector<std::pair<double,double> > scores;
    std::vector<SparseVector*> features;
    BOOST_FOREACH(const boost::shared_ptr<HyperPath> & path, nbest) {
        Sentence my_hyp = path->CalcTranslation(factor_).words;
        std::pair<double,double> score(path->GetScore(), -DBL_MAX);
        BOOST_FOREACH(const Sentence & ref, refs)
            score.second = std::max(
                            eval.CalculateStats(ref, src)->ConvertToScore(),
                            score.second);
        scores.push_back(score);
        features.push_back(new SparseVector(path->GetFeatures()));
    }
    AdjustNbest(scores,features);
    BOOST_FOREACH(SparseVector * feat, features)
        delete feat;
}
