
#include <travatar/eval-measure-bleu.h>
#include <travatar/util.h>
#include <boost/lexical_cast.hpp>
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
shared_ptr<EvalStats> EvalMeasureBleu::CalculateStats(const Sentence & ref, const Sentence & sys, int ref_cache_id, int sys_cache_id) {
    return CalculateStats(*GetCachedStats(ref, ref_cache_id), ref.size(), *GetCachedStats(sys, sys_cache_id), sys.size());
}

shared_ptr<EvalStats> EvalMeasureBleu::CalculateStats(const NgramStats & ref_ngrams, int ref_len,
                                                      const NgramStats & sys_ngrams, int sys_len) {
    int vals_n = 2*ngram_order_+1;
    vector<EvalStatsDataType> vals(vals_n);

    for (int i =0; i<ngram_order_; i++) {
        vals[2*i] = 0;
        vals[2*i+1] = max(sys_len-i,0);
    }

    for (NgramStats::const_iterator it = sys_ngrams.begin(); it != sys_ngrams.end(); it++) {
        NgramStats::const_iterator ref_it = ref_ngrams.find(it->first);
        if(ref_it != ref_ngrams.end()) {
            vals[2* (it->first.size()-1)] += min(ref_it->second,it->second);
        }
    }
    vals[vals_n-1] = ref_len;
    // Create the stats for this sentence
    EvalStatsPtr ret(new EvalStatsBleu(vals, smooth_val_));
    // If we are using sentence based, take the average immediately
    if(scope_ == SENTENCE)
        ret = EvalStatsPtr(new EvalStatsAverage(ret->ConvertToScore()));
    return ret;
}

BleuReport EvalStatsBleu::CalcBleuReport() const {
    BleuReport report;
    int ngram_order = (vals_.size()-1)/2;
    double log_bleu = 0.0;
    // Calculate the precision for each order
    for (int i=0; i < ngram_order; i++) {
        double smooth = (i == 0 ? 0 : smooth_);
        double num = (vals_[2*i]+smooth);
        double denom = (vals_[2*i+1]+smooth);
        double prec = (denom ? num/denom : 0);
        // cerr << "i="<<i<<", num="<<num<<", denom="<<denom<<", prec="<<prec<<endl;
        report.precs.push_back(prec);
        log_bleu += (prec ? log(prec) : -DBL_MAX);
    }
    log_bleu /= ngram_order;
    // vals_[vals__n-1] is the ref length, vals_[1] is the test length
    report.ref_len = vals_[vals_.size()-1];
    report.sys_len = vals_[1];
    // Calculate the brevity penalty
    report.ratio = (double)report.sys_len/report.ref_len;
    double log_bp = 1.0-(double)report.ref_len/report.sys_len;
    if (log_bp < 0.0) {
        log_bleu += log_bp;
        report.brevity = exp(log_bp);
    } else {
        report.brevity = 1.0;
    }
    // Sanity check
    if(log_bleu > 0) THROW_ERROR("Found a BLEU larger than one: " << exp(log_bleu))
    report.bleu = exp(log_bleu);
    return report;
}


std::string EvalStatsBleu::ConvertToString() const {
    BleuReport report = CalcBleuReport();
    ostringstream oss;
    oss << "BLEU = " << report.bleu << ", " << SafeAccess(report.precs, 0);
    for(int i = 1; i < (int)report.precs.size(); i++)
        oss << "/" << report.precs[i];
    oss << " (BP=" << report.brevity << ", ratio=" << report.ratio << ", hyp_len=" << report.sys_len << ", ref_len=" << report.ref_len << ")";
    return oss.str();
}

double EvalStatsBleu::ConvertToScore() const {
    return CalcBleuReport().bleu;
}


EvalMeasureBleu::EvalMeasureBleu(const std::string & config) : ngram_order_(4), smooth_val_(0), scope_(CORPUS) {
    if(config.length() == 0) return;
    BOOST_FOREACH(const EvalMeasure::StringPair & strs, EvalMeasure::ParseConfig(config)) {
        if(strs.first == "order") {
            ngram_order_ = boost::lexical_cast<int>(strs.second);
        } else if(strs.first == "smooth") {
            smooth_val_ = boost::lexical_cast<double>(strs.second);
        } else if(strs.first == "scope") {
            if(strs.second == "corpus") {
                scope_ = CORPUS;
            } else if(strs.second == "sentence") {
                scope_ = SENTENCE;
            } else {
                THROW_ERROR("Bad BLEU scope: " << config);
            }
        } else {
            THROW_ERROR("Bad configuration string: " << config);
        }
    }
}
