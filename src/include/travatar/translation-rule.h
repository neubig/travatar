#ifndef TRANSLATION_RULE_H__
#define TRANSLATION_RULE_H__

#include <travatar/sentence.h>
#include <travatar/cfg-data.h>
#include <travatar/sparse-map.h>
#include <string>
#include <vector>

namespace travatar {

class TranslationRule {

public:
    TranslationRule(// const std::string & src_str = "",
                    const CfgDataVector & trg_data = CfgDataVector(),
                    const SparseVector & features = SparseVector()) :
        // src_str_(src_str), 
        trg_data_(trg_data), features_(features) { }

    virtual ~TranslationRule() {} 

    void AddFeature(int id, double feat);
    void AddFeature(const std::string & str, double feat);
    
    virtual bool operator==(const TranslationRule & rhs) const {
        return
            // src_str_ == rhs.src_str_ &&
            trg_data_ == rhs.trg_data_ &&
            features_ == rhs.features_;
    }
    bool operator!=(const TranslationRule & rhs) const {
        return !(*this == rhs);
    }

    void Print(std::ostream & out) const;

    // const std::string & GetSrcStr() const { return src_str_; }
    const CfgDataVector & GetTrgData() const { return trg_data_; }
    const SparseVector & GetFeatures() const { return features_; }
    // std::string & GetSrcStr() { return src_str_; }
    CfgDataVector & GetTrgData() { return trg_data_; }
    SparseVector & GetFeatures() { return features_; }

    void AddTrgWord(WordId word, int factor = 0) {
        if(factor <= (int)trg_data_.size())
            trg_data_.resize(factor+1);
        trg_data_[factor].words.push_back(word);
    }
    void AddTrgSym(WordId sym, int factor = 0) {
        if(factor <= (int)trg_data_.size())
            trg_data_.resize(factor+1);
        trg_data_[factor].syms.push_back(sym);
    }
    void SetTrgLabel(WordId lab, int factor = 0) {
        if(factor <= (int)trg_data_.size())
            trg_data_.resize(factor+1);
        trg_data_[factor].label = lab;
    }

protected:
    // std::string src_str_;
    CfgDataVector trg_data_;
    SparseVector features_;

};
inline std::ostream &operator<<( std::ostream &out, const TranslationRule &L ) {
    L.Print(out);
    return out;
}

}

#endif
