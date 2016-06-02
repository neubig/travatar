
#include <travatar/eval-measure-nist.h>
#include <travatar/global-debug.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <cmath>

using namespace std;
using namespace travatar;
using namespace boost;

void EvalMeasureNist::ExtractNgrams(const Sentence & sentence, EvalMeasureNist::NgramStats & all_ngrams) const {
    vector<WordId> ngram;
    for (int k = 0; k < ngram_order_; k++) {
        for(int i =0; i < max((int)sentence.size()-k,0); i++) {
            for ( int j = i; j<= i+k; j++) {
                ngram.push_back(sentence[j]);
            }
            ++all_ngrams[ngram];
            ngram.clear();
        }
    }
}

// Initialize with reference
void EvalMeasureNist::InitializeWithReferences(const std::vector< std::vector<Sentence> > & refs) {
    NgramStats counts;
    typedef std::vector<Sentence> Sentences;
    int null = 0;
    BOOST_FOREACH(const Sentences & sents, refs) {
        const Sentence & sent = sents[factor_];
        null += sent.size();
        ExtractNgrams(sent, counts);
    }
    weight_ngrams_.clear();
    Real log2 = log(2.0);
    BOOST_FOREACH(const NgramStats::value_type & val, counts) {
        if(val.first.size() == 1) {
            weight_ngrams_[val.first] = log((Real)val.second/null)/log2;
        } else {
            vector<WordId> context = val.first; context.resize(val.first.size()-1);
            NgramStats::const_iterator it = counts.find(context);
            if(it == counts.end()) THROW_ERROR("Couldn't find context");
            weight_ngrams_[val.first] = log((Real)val.second/it->second)/log2;
        }
    }
}

boost::shared_ptr<EvalStats> EvalMeasureNist::CalculateStats(const Sentence & ref, const Sentence & sys) const {
    NgramStats ref_s, sys_s;
    ExtractNgrams(ref, ref_s);
    ExtractNgrams(sys, sys_s);
    return CalculateStats(ref_s, ref.size(), sys_s, sys.size());
}

boost::shared_ptr<EvalStats> EvalMeasureNist::CalculateStats(const NgramStats & ref_ngrams, int ref_len,
                                                             const NgramStats & sys_ngrams, int sys_len) const {
    int vals_n = 2*ngram_order_;
    vector<EvalStatsDataType> vals(vals_n+1);

    for (int i =0; i<ngram_order_; i++) {
        vals[2*i] = 0;
        vals[2*i+1] = max(sys_len-i,0);
    }
    vals[vals_n] = ref_len;

    for (NgramStats::const_iterator it = sys_ngrams.begin(); it != sys_ngrams.end(); it++) {
        NgramStats::const_iterator ref_it = ref_ngrams.find(it->first);
        if(ref_it != ref_ngrams.end()) {
            NgramWeights::const_iterator weight_it = weight_ngrams_.find(it->first);
            if(weight_it == weight_ngrams_.end()) THROW_ERROR("n-gram found in reference, but not in cached weights for NIST");
            vals[2* (it->first.size()-1)] += min(ref_it->second,it->second) * weight_it->second;
        }
    }
    // Create the stats for this sentence
    EvalStatsPtr ret(new EvalStatsNist(vals, beta_));
    // If we are using sentence based, take the average immediately
    if(scope_ == SENTENCE_NIST)
        ret = EvalStatsPtr(new EvalStatsAverage(ret->ConvertToScore()));
    return ret;
}

// Read in the stats
boost::shared_ptr<EvalStats> EvalMeasureNist::ReadStats(const std::string & line) {
    EvalStatsPtr ret;
    if(scope_ == SENTENCE_NIST)
        ret.reset(new EvalStatsAverage);
    else
        ret.reset(new EvalStatsNist(std::vector<EvalStatsDataType>(), beta_));
    ret->ReadStats(line);
    return ret;
}

NistReport EvalStatsNist::CalcNistReport() const {
    NistReport report;
    int ngram_order = GetNgramOrder();
    // Calculate the precision for each order
    for (int i=0; i < ngram_order; i++) {
        Real score = vals_[2*i]/vals_[2*i+1];
        report.scores.push_back(score);
        report.nist += score;
    }

    // vals_.rbegin is the ref length, vals_[1] is the test length
    report.sys_len = vals_[1];
    report.ref_len = *vals_.rbegin();
    // Calculate the brevity penalty
    report.ratio = (Real)report.sys_len/report.ref_len;
    if(report.ratio >= 1.0) {
      report.brevity = 1.0;
    } else if(report.ratio <= 0.0) {
      report.brevity = 0.0;
    } else {
      Real adj_ratio = log(min(report.ratio,1.0));
      report.brevity = exp(-beta_*adj_ratio*adj_ratio);
    }
    report.nist *= report.brevity;
    return report;
}


std::string EvalStatsNist::ConvertToString() const {
    NistReport report = CalcNistReport();
    ostringstream oss;
    oss << GetIdString() << " = " << report.nist << ", " << report.scores[0];
    for(int i = 1; i < (int)report.scores.size(); i++)
        oss << "/" << report.scores[i];
    oss << " (BP=" << report.brevity << ", ratio=" << report.ratio << ", hyp_len=" << report.sys_len << ", ref_len=" << report.ref_len << ")";
    return oss.str();
}

Real EvalStatsNist::ConvertToScore() const {
    return CalcNistReport().nist;
}


EvalMeasureNist::EvalMeasureNist(const std::string & config) : ngram_order_(5), scope_(CORPUS_NIST), beta_(NIST_BETA_VALUE) {
    if(config.length() == 0) return;
    BOOST_FOREACH(const EvalMeasure::StringPair & strs, EvalMeasure::ParseConfig(config)) {
        if(strs.first == "order") {
            ngram_order_ = boost::lexical_cast<int>(strs.second);
        } else if(strs.first == "beta") {
            beta_ = boost::lexical_cast<Real>(strs.second);
        } else if(strs.first == "scope") {
            if(strs.second == "corpus") {
                scope_ = CORPUS_NIST;
            } else if(strs.second == "sentence") {
                scope_ = SENTENCE_NIST;
            } else {
                THROW_ERROR("Bad NIST scope: " << config);
            }
        } else {
            THROW_ERROR("Bad configuration string: " << config);
        }
    }
}
