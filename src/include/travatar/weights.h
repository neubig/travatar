#ifndef WEIGHTS_H__
#define WEIGHTS_H__

#include <travatar/sparse-map.h>
#include <travatar/nbest-list.h>
#include <travatar/sentence.h>
#include <travatar/util.h>
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
    virtual void SetCurrent(const SparseMap & kv) { current_ = kv; }

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        return current_;
    }

    // Whether to adjust weights or not. If this is false, Adjust will throw
    // an error, but if it is true adjust will adjust the weights in an online
    // fashion
    virtual bool DoAdjust() {
        return false;
    }

    // Adjust the weights according to the n-best list
    // Scores are current model scores and evaluation scores
    virtual void Adjust(
            const std::vector<std::pair<double,double> > & scores,
            const std::vector<SparseMap*> & features) {
        THROW_ERROR("Standard weights cannot be adjusted");
    }

    // Adjust based on a single one-best list
    virtual void Adjust(const Sentence & src,
                        const std::vector<Sentence> & refs,
                        const EvalMeasure & eval,
                        const NbestList & nbest);

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
