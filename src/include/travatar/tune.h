#ifndef TUNE_H__
#define TUNE_H__

#include <travatar/real.h>
#include <travatar/sparse-map.h>
#include <travatar/sentence.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <cfloat>

namespace travatar {

class TuningExample;

// Performs MERT
class Tune {

public:

    // **** Non-static Members ****
    Tune();

    // Tune weights
    virtual Real RunTuning(SparseMap & weights) = 0;

    // Initialize any parameters
    virtual void Init(const SparseMap & init_weights) { }

    // Find gradient range
    std::pair<Real,Real> FindGradientRange(
                                const SparseMap & weights,
                                WordId feat);
    std::pair<Real,Real> FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                std::pair<Real,Real> range);

    // Getters/Setters
    void SetExamples(const std::vector<boost::shared_ptr<TuningExample> > & examps) { examps_ = examps; }
    int NumExamples() { return examps_.size(); }
    void SetGainThreshold(Real thresh) { gain_threshold_ = thresh; }
    Real GetGainThreshold() { return gain_threshold_; }
    void SetRange(int id, Real min, Real max) {
        ranges_[id] = std::pair<Real,Real>(min,max);
    }
    void AddExample(const boost::shared_ptr<TuningExample> & examp) {
        examps_.push_back(examp);
    }
    std::vector<boost::shared_ptr<TuningExample> > & GetExamples() { return examps_; }
    const std::vector<boost::shared_ptr<TuningExample> > & GetExamples() const { return examps_; }
    TuningExample & GetExample(int id);
    WordId GetScaleId() { return scale_id_; }

protected:

    // A feature must create a gain of more than this to be added
    Real gain_threshold_;

    // A special word ID that corresponds to the scaling factor
    WordId scale_id_;

    // The range of the weights
    typedef boost::unordered_map<WordId, std::pair<Real,Real> > RangeMap;
    RangeMap ranges_;

    // The examples to use
    std::vector<boost::shared_ptr<TuningExample> > examps_;

};

}

#endif
