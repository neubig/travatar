#ifndef EVAL_MEASURE_NIST_H__
#define EVAL_MEASURE_NIST_H__

// This class calculate the NIST evaulation measure as proposed by
//  Automatic Evaluation of Machine Translation Quality Using N-gram Co-Occurrence Statistics
//  by NIST

#include <travatar/eval-measure.h>
#include <travatar/real.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

// Beta is set so that brevity penalty is 0.5 when system output is 2/3 of reference ln(1.5)/ln(0.5)/ln(0.5)
#define NIST_BETA_VALUE 4.21617361683

namespace travatar {

// Whether to use corpus-based or sentence-by-sentence NIST
typedef enum { CORPUS_NIST, SENTENCE_NIST } NistScope;

typedef struct {
    Real nist; // NIST score
    std::vector<Real> scores; // Information gain for each n-gram level
    Real ref_len, sys_len; // Reference and system, lengths
    Real ratio, brevity; // The ref/sys ratio and brevity penalty
} NistReport;

class EvalStatsNist : public EvalStats {
public:
    EvalStatsNist(const std::vector<EvalStatsDataType> vals = std::vector<EvalStatsDataType>(),
                  Real beta = NIST_BETA_VALUE) 
            : beta_(beta) {
        vals_ = vals;
    }
    virtual std::string GetIdString() const { return "NIST"; }
    virtual Real ConvertToScore() const;
    virtual std::string ConvertToString() const;
    virtual EvalStatsPtr Clone() const { return EvalStatsPtr(new EvalStatsNist(vals_, beta_)); }
    NistReport CalcNistReport() const;
    int GetNgramOrder() const { return vals_.size()/2; }
private:
    Real beta_;
};

class EvalMeasureNist : public EvalMeasure {

public:

    // NgramStats are a mapping between ngrams and the number of occurrences
    typedef std::map<std::vector<WordId>,int> NgramStats;
    typedef std::map<std::vector<WordId>,Real> NgramWeights;

    EvalMeasureNist(int ngram_order = 5, NistScope scope = CORPUS_NIST, Real beta = NIST_BETA_VALUE) : 
        ngram_order_(ngram_order), scope_(scope), beta_(beta) { }
    EvalMeasureNist(const std::string & config);

    // Initialize with reference
    virtual void InitializeWithReferences(const std::vector< std::vector<Sentence> > & refs);
    
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

    int GetNgramOrder() const { return ngram_order_; }
    void SetNgramOrder(int ngram_order) { ngram_order_ = ngram_order; }
    std::string GetIdString() { return "NIST"; }
protected:

    // Calculate the n-gram statistics necessary for NIST in advance
    void ExtractNgrams(const Sentence & sentence, NgramStats & ngrams) const;    

    // The order of NIST n-grams
    int ngram_order_;
    // The scope
    NistScope scope_;
    // The weighting for the brevity penalty
    Real beta_;

    // The weights of each n-gram
    NgramWeights weight_ngrams_;

};

}

#endif
