#ifndef EVAL_MEASURE_BLEU_H__
#define EVAL_MEASURE_BLEU_H__

#include <map>
#include <vector>
#include <travatar/eval-measure.h>

namespace travatar {

class EvalMeasureBleu : public EvalMeasure {

public:
    // NgramStats are a mapping between ngrams and the number of occurrences
    typedef std::map<std::vector<WordId>,int> NgramStats;

    EvalMeasureBleu() : ngram_order_(4), smooth_val_(1.0) { }

    // Measure the score of the system output according to the reference
    virtual double MeasureScore(
            const Sentence & reference,
            const Sentence & system) const;

    // Evaluate BLEU with pre-computed n-gram stats to save time
    double MeasureScore(const NgramStats & ref_ngrams,
                        int ref_len,
                        const NgramStats & sys_ngrams,
                        int sys_len) const;

    // Calculate the n-gram statistics necessary for BLEU in advance
    NgramStats ExtractNgrams(const Sentence & sentence) const;

    int GetNgramOrder() const { return ngram_order_; }
    void SetNgramOrder(int ngram_order) { ngram_order_ = ngram_order; }
    double GetSmoothVal() const { return smooth_val_; }
    void SetSmoothVal(double smooth_val) { smooth_val_ = smooth_val; }

protected:
    // The order of BLEU n-grams
    int ngram_order_;
    // The amount by which to smooth n-grams over 1
    double smooth_val_;

};

}

#endif
