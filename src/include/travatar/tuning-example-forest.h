#ifndef TUNING_EXAMPLE_FOREST_H__
#define TUNING_EXAMPLE_FOREST_H__

#include <travatar/real.h>
#include <travatar/tuning-example.h>
#include <travatar/sentence.h>
#include <boost/shared_ptr.hpp>
#include <set>
#include <cfloat>

namespace travatar {

class HyperGraph;
class EvalMeasure;
class MertHull;
class MertHullWeightFunction;

class TuningExampleForest : public TuningExample {

public:

    // Constructor tanking the forest and the reference
    TuningExampleForest(EvalMeasure * measure,
                        const std::vector<Sentence> & refs,
                        int id,
                        Real mult) :
                            TuningExample(),
                            measure_(measure),
                            refs_(refs), oracle_score_(mult),
                            curr_score_(-REAL_MAX), id_(id), mult_(mult) {
    }

    virtual ~TuningExampleForest() { }

    // Find the featutres that are active in this forest
    void FindActiveFeatures();

    // Calculate the oracle of this sentence
    void CalculateOracle();

    // Calculate the gain that could be achieved by each feature
    // for this particular forest (oracle-current best)
    virtual SparseMap CalculatePotentialGain(const SparseMap & weights);

    // Count weights
    virtual void CountWeights(std::set<WordId> & weights);

    // Calculate the convex hull for this example given the current weights
    // and gradients
    virtual ConvexHull CalculateConvexHull(
                                const SparseMap & weights,
                                const SparseMap & gradient) const;
    
    // Add a forest hypothesis
    void AddHypothesis(const boost::shared_ptr<HyperGraph> & forest);


    // Calculate the n-best list giving the current weights
    virtual const std::vector<ExamplePair> & 
                       CalculateNbest(const Weights & weights);

    // Calculate the n-best list giving the current weights
    virtual const ExamplePair & 
                       CalculateModelHypothesis(Weights & weights) const;

protected:
    
    std::vector<ExamplePair> last_nbest_;

    // Recursively calculate the MERT hull
    const MertHull & CalculateMertHull(
                            const MertHullWeightFunction & func,
                            std::vector<boost::shared_ptr<MertHull> > & hulls, 
                            int node_id) const;

    EvalMeasure * measure_;
    boost::shared_ptr<HyperGraph> forest_;
    std::vector<Sentence> refs_;
    // The score that the best hypothesis in the forest achieves
    Real oracle_score_;
    // The score that the forest achieves with the current weights
    Real curr_score_;
    // A set of features that are active in the forest
    std::set<int> active_;
    // The ID of this example
    int id_;
    // Multiplier
    Real mult_;

};

}

#endif
