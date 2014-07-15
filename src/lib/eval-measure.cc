#include <boost/foreach.hpp>
#include <map>
#include <fstream>
#include <lm/model.hh>
#include <travatar/hyper-graph.h>
#include <travatar/eval-measure.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/eval-measure-ter.h>
#include <travatar/eval-measure-wer.h>
#include <travatar/eval-measure-interp.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <travatar/weights.h>
#include <travatar/lm-composer-bu.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace travatar;
using namespace boost;

#define NGRAM_ORDER 5
#define NBEST_COUNT 10
#define POP_LIMIT 500

bool EvalStats::IsZero() {
    BOOST_FOREACH(const EvalStatsDataType & val, vals_)
        if(val != 0)
            return false;
    return true;
}
// Utility functions
std::string EvalStats::ConvertToString() const {
    std::ostringstream oss;
    oss << GetIdString() << " = " << ConvertToScore();
    return oss.str();
}
EvalStats & EvalStats::PlusEquals(const EvalStats & rhs) {
    if(vals_.size() == 0) {
        vals_ = rhs.vals_;
    } else if (rhs.vals_.size() != 0) {
        if(rhs.vals_.size() != vals_.size())
            THROW_ERROR("Mismatched in EvalStats::PlusEquals");
        for(int i = 0; i < (int)rhs.vals_.size(); i++)
            vals_[i] += rhs.vals_[i];
    }
    return *this;
}
EvalStats & EvalStats::PlusEqualsTimes(const EvalStats & rhs, double p) {
    if(vals_.size() == 0) {
        vals_ = rhs.vals_;
        for(int i = 0; i < (int)vals_.size(); i++)
            vals_[i] *= p;
    } else if (rhs.vals_.size() != 0) {
        if(rhs.vals_.size() != vals_.size())
            THROW_ERROR("Mismatched in EvalStats::PlusEqualsTimes");
        for(int i = 0; i < (int)rhs.vals_.size(); i++)
            vals_[i] += rhs.vals_[i] * p;
    }
    return *this;
}
EvalStats & EvalStats::TimesEquals(EvalStatsDataType mult) {
    BOOST_FOREACH(EvalStatsDataType & val, vals_)
        val *= mult;
    return *this;
}
EvalStatsPtr EvalStats::Plus(const EvalStats & rhs) {
    EvalStatsPtr ret(this->Clone());
    ret->PlusEquals(rhs);
    return ret;
}
EvalStatsPtr EvalStats::Times(EvalStatsDataType mult) {
    EvalStatsPtr ret(this->Clone());
    ret->TimesEquals(mult);
    return ret;
}
bool EvalStats::Equals(const EvalStats & rhs) const {
    if(vals_.size() != rhs.vals_.size()) return false;
    for(int i = 0; i < (int)vals_.size(); i++) {
        if(fabs(vals_[i]-rhs.vals_[i]) > 1e-6)
            return false;
    }
    return true;
}
const std::vector<EvalStatsDataType> & EvalStats::GetVals() const { return vals_; }
void EvalStats::ReadStats(const std::string & str) {
    vals_.resize(0);
    EvalStatsDataType val;
    std::istringstream iss(str);
    while(iss >> val)
        vals_.push_back(val);
}
std::string EvalStats::WriteStats() {
    std::ostringstream oss;
    for(int i = 0; i < (int)vals_.size(); i++) {
        if(i) oss << ' ';
        oss << vals_[i];
    }
    return oss.str();
}


// Find the oracle sentence for this evaluation measure
Sentence EvalMeasure::CalculateOracle(const HyperGraph & graph, const Sentence & ref, int factor) {
    Sentence bord_ref;
    // Create the bordered sentence
    bord_ref.push_back(Dict::WID("<s>"));
    BOOST_FOREACH(WordId wid, ref) bord_ref.push_back(wid);
    bord_ref.push_back(Dict::WID("</s>"));
    // Create the n-grams
    typedef map<Sentence, int> NgramMap;
    vector<NgramMap> ngrams(NGRAM_ORDER+1);
    int act_order = 0;
    for(int i = 0; i < (int)bord_ref.size(); i++) {
        Sentence curr;
        for(int j = 0; j <= NGRAM_ORDER; j++) {
            ngrams[j][curr]++;
            act_order = max(j, act_order);
            if(i+j >= (int)bord_ref.size()) break;
            curr.push_back(bord_ref[i+j]);
        }
    }
    // Write the arpa file
    ofstream ofs("/tmp/oracle.arpa");
    if(!ofs) THROW_ERROR("Could not open /tmp/oracle.arpa for writing");
    ofs << "\\data\\\n";
    for(int n = 1; n <= act_order; n++) {
        int size = ngrams[n].size() + (n == 1 ? 1 : 0);
        ofs << "ngram " << n << "=" << size << endl;
    }
    for(int n = 1; n <= act_order; n++) {
        if(n != 1 && ngrams[n].size() == 0) break;
        ofs << endl << "\\"<<n<<"-grams:"<<endl;
        if(n == 1) ofs << "-99\t<unk>\t-99" << endl;
        BOOST_FOREACH(const NgramMap::value_type & val, ngrams[n]) {
            Sentence context = val.first;
            context.resize(context.size()-1);
            ofs << log(val.second)-log(ngrams[n-1][context]) << "\t" << Dict::PrintWords(val.first);
            if(n != act_order) ofs << "\t-99";
            ofs << endl;
        }
    }
    ofs << "\\end\\" << endl << endl;
    // Create the LM, and an index mapping from travatar IDs to 
    MapEnumerateVocab lm_save;
    lm::ngram::Config lm_config;
    lm_config.enumerate_vocab = &lm_save;
    lm_config.messages = NULL;
    lm::ngram::Model* lm_model = new lm::ngram::Model("/tmp/oracle.arpa", lm_config);
    LMComposerBU bu(lm_model, lm_save.GetAndFreeVocabMap());
    bu.GetData()[0]->SetFeatureName(Dict::WID("oraclelm"));
    bu.GetData()[0]->SetWeight(1);
    bu.SetStackPopLimit(POP_LIMIT);
    // Decode with the reference
    HyperGraph rescored_graph(graph);
    Weights empty_weights;
    rescored_graph.ScoreEdges(empty_weights);
    shared_ptr<HyperGraph> lm_graph(bu.TransformGraph(rescored_graph));
    // Create n-best list
    NbestList nbest_list = lm_graph->GetNbest(NBEST_COUNT);
    // Find the sentence in the n-best list with the highest score
    Sentence ret; 
    double best_score = 0;
    BOOST_FOREACH(const shared_ptr<HyperPath> & path, nbest_list) {
        Sentence curr = path->GetTrgData()[factor].words;
        double score = this->CalculateStats(ref, curr)->ConvertToScore();
        if(score > best_score) {
            ret = curr;
            best_score = score;
        }
    }
    // Return the sentence
    return ret;
}

EvalMeasure * EvalMeasure::CreateMeasureFromString(const string & str) {
    // Get the eval, config substr
    string eval, config;
    size_t eq = str.find(':');
    if(eq == string::npos) { eval = str; }
    else { eval = str.substr(0,eq); config = str.substr(eq+1); }
    // Create the actual measure
    if(eval == "bleu") 
        return new EvalMeasureBleu(config);
    else if(eval == "ribes")
        return new EvalMeasureRibes(config);
    else if(eval == "ter")
        return new EvalMeasureTer(config);
    else if(eval == "wer")
        return new EvalMeasureWer(config);
    else if(eval == "interp")
        return new EvalMeasureInterp(config);
    else
        THROW_ERROR("Unknown evaluation measure: " << eval);
    return NULL;
}

vector<EvalMeasure::StringPair> EvalMeasure::ParseConfig(const string & str) {
    vector<string> arr1, arr2;
    boost::split ( arr1, str, boost::is_any_of(","));
    vector<EvalMeasure::StringPair> ret;
    BOOST_FOREACH(const std::string & my_str, arr1) {
        boost::split ( arr2, my_str, boost::is_any_of("="));
        if(arr2.size() != 2)
            THROW_ERROR("Bad evaluation measure config:" << str);
        ret.push_back(make_pair(arr2[0], arr2[1]));
    }
    return ret;
}
