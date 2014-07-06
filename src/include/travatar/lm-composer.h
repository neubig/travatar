#ifndef LM_COMPOSER_H__
#define LM_COMPOSER_H__

#include <string>
#include <utility>
#include <tr1/unordered_map>
#include <lm/left.hh>
#include <lm/model.hh>
#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>

namespace travatar {

// A map from Travatar vocab to KenLM vocab
typedef std::tr1::unordered_map<WordId, lm::WordIndex> VocabMap;

class MapEnumerateVocab : public lm::EnumerateVocab {
public:
    MapEnumerateVocab() : EnumerateVocab(), vocab_map_(new VocabMap) { }
    virtual ~MapEnumerateVocab() {
        if(vocab_map_)
            delete vocab_map_;    
    }

    virtual void Add(lm::WordIndex index, const StringPiece &str) {
        vocab_map_->insert(std::make_pair(Dict::WID(str.as_string()), index));
    }

    std::tr1::unordered_map<WordId, lm::WordIndex> * GetAndFreeVocabMap() {
        VocabMap * ret = vocab_map_;
        vocab_map_ = NULL;
        return ret;
    }

protected:
    VocabMap * vocab_map_;
};

// The data for each LM
// This is read from a specification string of the following format
//   /path/to/file.blm|factor=0,lm_feat=lm,lm_unk_feat=lmunk
// where the first string is the file and the following parameters are optional
//   factor: which factor to use
//   lm_feat: the name of the feature for the lm probability
//   lm_unk: the name of the feature for LM unknown words
class LMData {
public:
    LMData(const std::string & str);
    LMData(lm::ngram::Model * model, VocabMap* vocab_map) :
        lm_feat_(Dict::WID("lm")), lm_unk_feat_(Dict::WID("lmunk")), 
        lm_(model), vocab_map_(vocab_map), 
        lm_weight_(1), lm_unk_weight_(0), factor_(0) { }

    virtual ~LMData() {
        if(lm_) delete lm_;
        if(vocab_map_) delete vocab_map_;
    }

    lm::WordIndex GetMapping(WordId wid) const {
        VocabMap::const_iterator it = vocab_map_->find(wid);
        return it == vocab_map_->end() ? 0 : it->second;
    }

    lm::ngram::Model * GetLM() { return lm_; }
    double GetWeight() const { return lm_weight_; }
    void SetWeight(double lm_weight) { lm_weight_ = lm_weight; }
    double GetUnkWeight() const { return lm_unk_weight_; }
    void SetUnkWeight(double lm_unk_weight) { lm_unk_weight_ = lm_unk_weight; }
    WordId GetFeatureName() const { return lm_feat_; }
    void SetFeatureName(WordId lm_feat) { lm_feat_ = lm_feat; }
    WordId GetUnkFeatureName() const { return lm_unk_feat_; }
    void SetUnkFeatureName(WordId lm_unk_feat) { lm_unk_feat_ = lm_unk_feat; }
    int GetFactor() const { return factor_; }
    void SetFactor(int factor) { factor_ = factor; }

protected:
    // The name of the feature expressed by this model
    WordId lm_feat_, lm_unk_feat_;
    // The language model that this composer handles
    lm::ngram::Model * lm_;
    // The vocabulary map from Travatar vocab to LM vocab
    VocabMap * vocab_map_;
    // The weight assigned to this particular LM
    double lm_weight_, lm_unk_weight_;
    // The factor to use
    int factor_; 
};

// A parent class for search algorithms that compose a rule graph with a
// target side language model
class LMComposer : public GraphTransformer {
    
protected:
    std::vector<LMData*> lm_data_;

public:
    LMComposer(const std::string & str);
    LMComposer(lm::ngram::Model * model, VocabMap* vocab_map) {
        LMData * data = new LMData(model,vocab_map);
        lm_data_.push_back(data);
    }
        
    std::vector<LMData*> & GetData() { return lm_data_; }
    const std::vector<LMData*> & GetData() const { return lm_data_; }

    virtual ~LMComposer() {
        BOOST_FOREACH(LMData* data, lm_data_)
            if(data) delete data;
    }

    void UpdateWeights(const SparseMap & weights) {
        BOOST_FOREACH(LMData* data, lm_data_) {
            SparseMap::const_iterator it1 = weights.find(data->GetFeatureName());
            data->SetWeight(it1 != weights.end() ? it1->second : 0);
            SparseMap::const_iterator it2 = weights.find(data->GetUnkFeatureName());
            data->SetUnkWeight(it2 != weights.end() ? it2->second : 0);
        }
    }

    // Compose the rule graph with a language model
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const = 0;

};

}

#endif
