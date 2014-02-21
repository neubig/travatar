#ifndef LM_COMPOSER_H__
#define LM_COMPOSER_H__

#include <string>
#include <lm/left.hh>
#include <lm/model.hh>
#include <travatar/graph-transformer.h>

namespace travatar {

// A parent class for search algorithms that compose a rule graph with a
// target side language model
class LMComposer : public GraphTransformer {

protected:
    // The name of the feature expressed by this model
    std::string lm_feature_name_, lm_unk_feature_name_;
    // The language model that this composer handles
    lm::ngram::Model * lm_;
    // The weight assigned to this particular LM
    double lm_weight_, lm_unk_weight_;

public:
    LMComposer() : lm_feature_name_("lm"), lm_unk_feature_name_("lmunk"), lm_(NULL), lm_weight_(1), lm_unk_weight_(1) { }
    LMComposer(lm::ngram::Model * lm) : lm_feature_name_("lm"), lm_unk_feature_name_("lmunk"), lm_(lm), lm_weight_(1), lm_unk_weight_(1) { }

    virtual ~LMComposer() {
        if(lm_) delete lm_;
    }

    // Compose the rule graph with a language model
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const = 0;

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
