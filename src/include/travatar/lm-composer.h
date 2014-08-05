#ifndef LM_COMPOSER_H__
#define LM_COMPOSER_H__

#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <lm/left.hh>
#include <lm/model.hh>
#include <tr1/unordered_map>
#include <string>
#include <utility>

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

    virtual void Add(lm::WordIndex index, const StringPiece &str);

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
    LMData(void * model, lm::ngram::ModelType type, VocabMap* vocab_map);

    virtual ~LMData();

    lm::WordIndex GetMapping(WordId wid) const;

    void * GetLM() { return lm_; }
    const void * GetLM() const { return lm_; }
    double GetWeight() const { return lm_weight_; }
    void SetWeight(double lm_weight) { lm_weight_ = lm_weight; }
    double GetUnkWeight() const { return lm_unk_weight_; }
    void SetUnkWeight(double lm_unk_weight) { lm_unk_weight_ = lm_unk_weight; }
    WordId GetFeatureName() const { return lm_feat_; }
    void SetFeatureName(WordId lm_feat) { lm_feat_ = lm_feat; }
    WordId GetUnkFeatureName() const { return lm_unk_feat_; }
    void SetUnkFeatureName(WordId lm_unk_feat) { lm_unk_feat_ = lm_unk_feat; }
    int GetFactor() const { return factor_; }
    lm::ngram::ModelType GetType() const { return type_; }
    void SetFactor(int factor) { factor_ = factor; }

protected:
    // The name of the feature expressed by this model
    WordId lm_feat_, lm_unk_feat_;
    // The language model that this composer handles
    void * lm_;
    // Identify the type of model being used
    lm::ngram::ModelType type_;
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
    LMComposer(void * model, lm::ngram::ModelType type, VocabMap* vocab_map) {
        LMData * data = new LMData(model, type, vocab_map);
        lm_data_.push_back(data);
    }
        
    std::vector<LMData*> & GetData() { return lm_data_; }
    const std::vector<LMData*> & GetData() const { return lm_data_; }

    virtual ~LMComposer();

    void UpdateWeights(const SparseMap & weights);

    // Compose the rule graph with a language model
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const = 0;

};

}

#endif
