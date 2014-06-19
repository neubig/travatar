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

// A parent class for search algorithms that compose a rule graph with a
// target side language model
class LMComposer : public GraphTransformer {

protected:
    // The name of the feature expressed by this model
    std::string lm_feature_name_, lm_unk_feature_name_;
    // The language model that this composer handles
    lm::ngram::Model * lm_;
    // The vocabulary map from Travatar vocab to LM vocab
    VocabMap * vocab_map_;
    // The weight assigned to this particular LM
    double lm_weight_, lm_unk_weight_;
    // The factor to use
    int factor_;

public:
    LMComposer() : lm_feature_name_("lm"), lm_unk_feature_name_("lmunk"), lm_(NULL), vocab_map_(NULL), lm_weight_(1), lm_unk_weight_(1), factor_(0) { }
    LMComposer(const std::string & str, int factor = 0) : lm_feature_name_("lm"), lm_unk_feature_name_("lmunk"), lm_weight_(1), lm_unk_weight_(1), factor_(factor) {
        // Create the LM, and an index mapping from travatar IDs to 
        MapEnumerateVocab lm_save;
        lm::ngram::Config lm_config;
        lm_config.enumerate_vocab = &lm_save;
        lm_ = new lm::ngram::Model(str.c_str(), lm_config);
        vocab_map_ = lm_save.GetAndFreeVocabMap();
    }
    LMComposer(lm::ngram::Model * lm, VocabMap * vocab_map, int factor = 0) : lm_feature_name_("lm"), lm_unk_feature_name_("lmunk"), lm_(lm), vocab_map_(vocab_map), lm_weight_(1), lm_unk_weight_(1), factor_(factor) { }

    virtual ~LMComposer() {
        if(lm_) delete lm_;
        if(vocab_map_) delete vocab_map_;
    }

    // Compose the rule graph with a language model
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const = 0;

    lm::WordIndex GetMapping(WordId wid) const {
        VocabMap::const_iterator it = vocab_map_->find(wid);
        return it == vocab_map_->end() ? 0 : it->second;
    }

    lm::ngram::Model * GetLM() { return lm_; }
    double GetWeight() const { return lm_weight_; }
    void SetWeight(double lm_weight) { lm_weight_ = lm_weight; }
    double GetUnkWeight() const { return lm_unk_weight_; }
    void SetUnkWeight(double lm_unk_weight) { lm_unk_weight_ = lm_unk_weight; }
    const std::string & GetFeatureName() const { return lm_feature_name_; }
    void SetFeatureName(const std::string & lm_feature_name) { lm_feature_name_ = lm_feature_name; }
    const std::string & GetUnkFeatureName() const { return lm_unk_feature_name_; }
    void SetUnkFeatureName(const std::string & lm_unk_feature_name) { lm_unk_feature_name_ = lm_unk_feature_name; }

};

}

#endif
