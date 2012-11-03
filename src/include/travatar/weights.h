#ifndef WEIGHTS_H__
#define WEIGHTS_H__

#include <travatar/sparse-map.h>
#include <travatar/eval-measure.h>
#include <travatar/hyper-graph.h>

namespace travatar {

class Weights {

public:

    Weights() { }
    Weights(const SparseMap & current) :
        current_(current) { }

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key) {
        return current_[key];
    }
    virtual void SetCurrent(const SparseMap::key_type & key, double val) {
        current_[key] = val;
    }

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        return current_;
    }

    // Adjust the weights according to the n-best list
    virtual void Adjust(const EvalMeasure & eval,
                        const std::vector<Sentence> & refs,
                        const NbestList & nbest) {
        // By default, do nothing
    }

protected:
    SparseMap current_;

};

double operator*(Weights & lhs, const SparseMap & rhs);

}

#endif
