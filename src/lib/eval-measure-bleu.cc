
#include <travatar/eval-measure-bleu.h>
#include <travatar/util.h>
#include <cmath>

using namespace std;
using namespace travatar;
using namespace boost;

EvalMeasureBleu::NgramStats * EvalMeasureBleu::ExtractNgrams(const Sentence & sentence) const {
    NgramStats * all_ngrams = new NgramStats;
    vector<WordId> ngram;
    for (int k = 0; k < ngram_order_; k++) {
        for(int i =0; i < max((int)sentence.size()-k,0); i++) {
            for ( int j = i; j<= i+k; j++) {
                ngram.push_back(sentence[j]);
            }
            ++((*all_ngrams)[ngram]);
            ngram.clear();
        }
    }
    return all_ngrams;
}

shared_ptr<EvalMeasureBleu::NgramStats> EvalMeasureBleu::GetCachedStats(const Sentence & sent, int cache_id) {
    if(cache_id == INT_MAX) return shared_ptr<NgramStats>(ExtractNgrams(sent));
    StatsCache::const_iterator it = cache_.find(cache_id);
    if(it == cache_.end()) {
        shared_ptr<NgramStats> new_stats(ExtractNgrams(sent));
        cache_.insert(make_pair(cache_id, new_stats));
        return new_stats;
    } else {
        return it->second;
    }
}

// Measure the score of the sys output according to the ref
double EvalMeasureBleu::MeasureScore(const Sentence & ref, const Sentence & sys, int ref_cache_id, int sys_cache_id) {
    return MeasureScore(*GetCachedStats(ref, ref_cache_id), ref.size(), *GetCachedStats(sys, sys_cache_id), sys.size());
}

double EvalMeasureBleu::MeasureScore(const NgramStats & ref_ngrams, int ref_len,
                                     const NgramStats & sys_ngrams, int sys_len) {

    int comps_n = 2*ngram_order_+1;
    vector<int> comps(comps_n);
    double log_bleu = 0.0, brevity;

    for (int i =0; i<ngram_order_; i++) {
        comps[2*i] = 0;
        comps[2*i+1] = max(sys_len-i,0);
    }

    for (NgramStats::const_iterator it = sys_ngrams.begin(); it != sys_ngrams.end(); it++) {
        NgramStats::const_iterator ref_it = ref_ngrams.find(it->first);
        if(ref_it != ref_ngrams.end()) {
            comps[2* (it->first.size()-1)] += min(ref_it->second,it->second);
        }
    }
    comps[comps_n-1] = ref_len;

    for (int i=0; i < ngram_order_; i++) {
        if (comps[0] == 0)
            return 0.0;
        if ( i > 0 )
            log_bleu += log((double)comps[2*i]+smooth_val_)-log((double)comps[2*i+1]+smooth_val_);
        else
            log_bleu += log((double)comps[2*i])-log((double)comps[2*i+1]);
    }
    log_bleu /= ngram_order_;
    brevity = 1.0-(double)comps[comps_n-1]/comps[1]; // comps[comps_n-1] is the ref length, comps[1] is the test length
    if (brevity < 0.0)
        log_bleu += brevity;
    // Sanity check
    if(log_bleu > 0) THROW_ERROR("Found a BLEU larger than one: " << exp(log_bleu))
    return exp(log_bleu);
}
