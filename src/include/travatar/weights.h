#ifndef WEIGHTS_H__
#define WEIGHTS_H__

#include <travatar/sparse-map.h>

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

protected:
    SparseMap current_;

};

double operator*(Weights & lhs, const SparseMap & rhs);

}

#endif
