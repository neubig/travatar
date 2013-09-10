#ifndef EVAL_MEASURE_BLEU_H__
#define EVAL_MEASURE_BLEU_H__

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <travatar/eval-measure.h>

namespace travatar {

typedef struct {
    double bleu; // BLEU score
    std::vector<double> precs; // n-gram precisions
    double ref_len, sys_len; // Reference and syste, lengths
    double ratio, brevity; // The ref/sys ratio and brevity penalty
} BleuReport;

class EvalStatsBleu : public EvalStats {
public:
    EvalStatsBleu(const std::vector<EvalStatsDataType> vals = std::vector<EvalStatsDataType>(), double smooth = 0)
                            : smooth_(smooth) {
        vals_ = vals;
    }
    virtual std::string GetIdString() const { return "BLEU"; }
    virtual double ConvertToScore() const;
    virtual std::string ConvertToString() const;
    virtual EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsBleu(vals_, smooth_)); }
    BleuReport CalcBleuReport() const;
    double GetAvgLogPrecision() const;
    double GetLengthRatio() const;
private:
    double smooth_;
};

class EvalMeasureBleu : public EvalMeasure {

public:

    // Whether to use corpus-based or sentence-by-sentence BLEU
    typedef enum { CORPUS, SENTENCE } BleuScope;

    // NgramStats are a mapping between ngrams and the number of occurrences
    typedef std::map<std::vector<WordId>,int> NgramStats;

    // A cache to hold the stats
    typedef std::map<int,boost::shared_ptr<NgramStats> > StatsCache;

    EvalMeasureBleu(int ngram_order = 4, double smooth_val = 0, BleuScope scope = CORPUS) : 
        ngram_order_(ngram_order), smooth_val_(smooth_val), scope_(scope) { }
    EvalMeasureBleu(const std::string & config);

    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys,
                int ref_cache_id = INT_MAX,
                int sys_cache_id = INT_MAX);
    
    // Calculate the stats for a single sentence
    virtual boost::shared_ptr<EvalStats> CalculateStats(
                const Sentence & ref,
                const Sentence & sys) const;

    // Calculate the stats for a single sentence
    virtual EvalStatsPtr ReadStats(
                const std::string & file);

    // Calculate the stats with cached n-grams
    boost::shared_ptr<EvalStats> CalculateStats(
                        const NgramStats & ref_ngrams,
                        int ref_len,
                        const NgramStats & sys_ngrams,
                        int sys_len) const; 

    // Calculate the n-gram statistics necessary for BLEU in advance
    NgramStats * ExtractNgrams(const Sentence & sentence) const;

    // Clear the ngram cache
    virtual void ClearCache() { cache_.clear(); }

    int GetNgramOrder() const { return ngram_order_; }
    void SetNgramOrder(int ngram_order) { ngram_order_ = ngram_order; }
    double GetSmoothVal() const { return smooth_val_; }
    void SetSmoothVal(double smooth_val) { smooth_val_ = smooth_val; }

protected:
    // The order of BLEU n-grams
    int ngram_order_;
    // The amount by which to smooth n-grams over 1
    double smooth_val_;
    // A cache to hold the stats
    StatsCache cache_;
    // The scope
    BleuScope scope_;

    // Get the stats that are in a cache
    boost::shared_ptr<NgramStats> GetCachedStats(const Sentence & sent, int cache_id);

};

}

#endif
