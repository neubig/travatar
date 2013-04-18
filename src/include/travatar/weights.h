#ifndef WEIGHTS_H__
#define WEIGHTS_H__

#include <travatar/sparse-map.h>
#include <travatar/nbest-list.h>
#include <travatar/sentence.h>
#include <cfloat>

namespace travatar {

class EvalMeasure;

class Weights {

public:

    Weights() {
        ranges_[-1] = std::pair<double,double>(-DBL_MAX, DBL_MAX);
    }
    Weights(const SparseMap & current) :
        current_(current) {
        ranges_[-1] = std::pair<double,double>(-DBL_MAX, DBL_MAX);
    }

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key) const {
        SparseMap::const_iterator it = current_.find(key);
        return (it != current_.end() ? it->second : 0.0);
    }
    virtual double GetCurrent(const SparseMap::key_type & key) {
        SparseMap::const_iterator it = current_.find(key);
        return (it != current_.end() ? it->second : 0.0);
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
        std::cerr << "Doing nothing!" << std::endl;
    }

    void SetRange(int id, double min, double max) {
        ranges_[id] = std::pair<double,double>(min,max);
    }
    std::pair<double,double> GetRange(int id) {
        RangeMap::const_iterator it = ranges_.find(id);
        return (it == ranges_.end() ? ranges_[-1] : it->second);
    }

protected:
    SparseMap current_;
    typedef std::tr1::unordered_map<WordId, std::pair<double,double> > RangeMap;
    RangeMap ranges_;

};

double operator*(Weights & lhs, const SparseMap & rhs);
double operator*(const Weights & lhs, const SparseMap & rhs);

}

#endif
