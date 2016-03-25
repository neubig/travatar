#ifndef LM_COMPOSER_H__
#define LM_COMPOSER_H__

#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <travatar/dict.h>
#include <travatar/real.h>
#include <travatar/lm-func.h>
#include <lm/left.hh>
#include <lm/model.hh>
#include <boost/unordered_map.hpp>
#include <string>
#include <utility>

namespace travatar {

// A parent class for search algorithms that compose a rule graph with a
// target side language model
class LMComposer : public GraphTransformer {
    
protected:
    std::vector<LMData*> lm_data_;
    WordId root_sym_;

public:
    LMComposer(const std::vector<std::string> & str);
    LMComposer(void * model, lm::ngram::ModelType type, VocabMap* vocab_map) {
        LMData * data = new LMData(model, type, vocab_map);
        lm_data_.push_back(data);
        root_sym_ = Dict::WID("LMROOT");
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
