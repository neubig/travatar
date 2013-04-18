#ifndef LM_COMPOSER_H__
#define LM_COMPOSER_H__

#include <string>
#include <lm/left.hh>
#include <travatar/graph-transformer.h>

namespace travatar {

// A parent class for search algorithms that compose a rule graph with a
// target side language model
class LMComposer : public GraphTransformer {

protected:
    // The name of the feature expressed by this model
    std::string feature_name_;
    // The language model that this composer handles
    lm::ngram::Model * lm_;
    // The weight assigned to this particular LM
    double lm_weight_;

public:
    LMComposer() : feature_name_("lm"), lm_(NULL), lm_weight_(1) { }
    LMComposer(lm::ngram::Model * lm) : feature_name_("lm"), lm_(lm), lm_weight_(1) { }

    virtual ~LMComposer() {
        if(lm_) delete lm_;
    }

    // Compose the rule graph with a language model
    virtual HyperGraph * TransformGraph(const HyperGraph & hg) const = 0;

    double GetLMWeight() const { return lm_weight_; }
    void SetLMWeight(double lm_weight) { lm_weight_ = lm_weight; }
    const std::string & GetFeatureName() const { return feature_name_; }
    void SetFeatureName(const std::string & feature_name) { feature_name_ = feature_name; }

};

}

#endif
