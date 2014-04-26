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

    Weights(int factor = 0) : factor_(factor_) {
        ranges_[-1] = std::pair<double,double>(-DBL_MAX, DBL_MAX);
    }
    Weights(const SparseMap & current, int factor = 0) :
        current_(current), factor_(factor) {
        ranges_[-1] = std::pair<double,double>(-DBL_MAX, DBL_MAX);
    }

    // Get the current values of the weights at this point in learning
    virtual double GetCurrent(const SparseMap::key_type & key) const {
        const SparseMap & current = GetCurrent();
        SparseMap::const_iterator it = current.find(key);
        return (it != current.end() ? it->second : 0.0);
    }
    virtual double GetCurrent(const SparseMap::key_type & key) {
        const SparseMap & current = GetCurrent();
        SparseMap::const_iterator it = current.find(key);
        return (it != current.end() ? it->second : 0.0);
    }
    virtual const SparseMap & GetCurrent() const {
        return current_;
    }
    virtual void SetCurrent(const SparseMap::key_type & key, double val) {
        current_[key] = val;
    }
    virtual void SetCurrent(const SparseMap & kv) { current_ = kv; }

    // Get the final values of the weights
    virtual const SparseMap & GetFinal() {
        return GetCurrent();
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

    // The pairwise weight update rule
    virtual void Update (
        const SparseMap & oracle, double oracle_score, double oracle_eval,
        const SparseMap & system, double system_score, double system_eval
    ) {
        THROW_ERROR("Standard weights cannot be updated");
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
    int factor_;

};

double operator*(Weights & lhs, const SparseMap & rhs);
double operator*(const Weights & lhs, const SparseMap & rhs);

}

#endif
