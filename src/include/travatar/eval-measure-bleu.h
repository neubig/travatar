#ifndef EVAL_MEASURE_BLEU_H__
#define EVAL_MEASURE_BLEU_H__

// This class calculate the BLEU evaulation measure as proposed by
//  BLEU: a method for automatic evaluation of machine translation
//  Papineni, Kishore and Roukos, Salim and Ward, Todd and Zhu, Wei-Jing
//  ACL 02
//
// It also implements some alternatives as mentioned in
//  BLEU deconstructed: Designing a Better MT Evaluation Metric
//  Xingyi Song, Trevor Cohn, Lucia Specia
//  CICLING 13

#include <travatar/eval-measure.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

namespace travatar {

// Whether to use corpus-based or sentence-by-sentence BLEU
typedef enum { CORPUS, SENTENCE } BleuScope;
typedef enum { GEOMETRIC, ARITHMETIC } BleuMean;

typedef struct {
    double bleu; // BLEU score
    std::vector<double> scores; // Scores for each n-gram (prec/rec/f)
    double ref_len, sys_len; // Reference and system, lengths
    double ratio, brevity; // The ref/sys ratio and brevity penalty
} BleuReport;

class EvalStatsBleu : public EvalStats {
public:
    EvalStatsBleu(const std::vector<EvalStatsDataType> vals = std::vector<EvalStatsDataType>(),
                  double smooth = 0,
                  double prec_weight = 1.0,
                  BleuMean mean = GEOMETRIC,
                  bool inverse = false,
                  bool calc_brev = true)
            : smooth_(smooth), prec_weight_(prec_weight), mean_(mean), inverse_(inverse), calc_brev_(calc_brev) {
        vals_ = vals;
    }
    virtual std::string GetIdString() const { return (inverse_ ? "INV_BLEU" : "BLEU"); }
    virtual double ConvertToScore() const;
    virtual std::string ConvertToString() const;
    virtual EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsBleu(vals_, smooth_, prec_weight_, mean_)); }
    BleuReport CalcBleuReport() const;
    double GetAvgLogPrecision() const;
    double GetLengthRatio() const;
    int GetNgramOrder() const { return vals_.size()/3; }
    double GetMatch(int n) const { return vals_[3*n]+smooth_; }
    double GetSysCount(int n) const { return vals_[3*n+1]+smooth_; }
    double GetRefCount(int n) const { return vals_[3*n+2]+smooth_; }
private:
    double smooth_;
    double prec_weight_;
    BleuMean mean_;
    // Flag
    bool inverse_;
    bool calc_brev_;
};

class EvalMeasureBleu : public EvalMeasure {

public:

    // NgramStats are a mapping between ngrams and the number of occurrences
    typedef std::map<std::vector<WordId>,int> NgramStats;

    // A cache to hold the stats
    typedef std::map<int,boost::shared_ptr<NgramStats> > StatsCache;

    EvalMeasureBleu(int ngram_order = 4, double smooth_val = 0,
                    BleuScope scope = CORPUS, double prec_weight = 1.0,
                    BleuMean mean = GEOMETRIC, bool inverse = false, bool calc_brevity = true) : 
        ngram_order_(ngram_order), smooth_val_(smooth_val), scope_(scope), prec_weight_(prec_weight), mean_(mean), inverse_(inverse), calc_brev_(calc_brevity) { }
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
    std::string GetIdString() { return (inverse_ ? "INV_BLEU" : "BLEU"); }
protected:
    // The order of BLEU n-grams
    int ngram_order_;
    // The amount by which to smooth n-grams over 1
    double smooth_val_;
    // A cache to hold the stats
    StatsCache cache_;
    // The scope
    BleuScope scope_;
    // The weight of precision in F-measure, from one to zero (default 1)
    double prec_weight_;
    // The type of mean, geometric or arithmetic (default geometric)
    BleuMean mean_;
    // Whether we are calculating the inverse of BLEU (PINC) or not
    bool inverse_;
    // Whether we want to add brevity penalty or not
    bool calc_brev_;

    // Get the stats that are in a cache
    boost::shared_ptr<NgramStats> GetCachedStats(const Sentence & sent, int cache_id);

};

}

#endif
