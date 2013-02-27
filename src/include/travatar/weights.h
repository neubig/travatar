#ifndef WEIGHTS_H__
#define WEIGHTS_H__

#include <travatar/sparse-map.h>
#include <travatar/nbest-list.h>
#include <travatar/sentence.h>

namespace travatar {

class EvalMeasure;

class Weights {

public:

    Weights() { }
    Weights(const SparseMap & current) :
        current_(current) { }

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key) {
        SparseMap::const_iterator it = current_.find(key);
        return (it != current_.end() ? current_[key] : 0.0);
    }
    virtual void SetCurrent(const SparseMap::key_type & key, double val) {
        current_[key] = val;
    }

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        return current_;
    }

    // Adjust the weights according to the n-best list
    virtual void Adjust(EvalMeasure & eval,
                        const std::vector<Sentence> & refs,
                        const NbestList & nbest) {
        // By default, do nothing
        std::cerr << "Doing nothing!" << std::endl;
    }

protected:
    SparseMap current_;

};

double operator*(Weights & lhs, const SparseMap & rhs);

}

#endif
