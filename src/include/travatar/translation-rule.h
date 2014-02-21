#ifndef TRANSLATION_RULE_H__
#define TRANSLATION_RULE_H__

#include <string>
#include <vector>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>

namespace travatar {

class TranslationRule {

public:
    TranslationRule(const std::string & src_str = "",
                    const std::vector<WordId> & trg_words = std::vector<WordId>(),
                    const std::vector<WordId> & trg_syms = std::vector<WordId>(),
                    const SparseMap & features = SparseMap()) :
        src_str_(src_str), trg_words_(trg_words), trg_syms_(trg_syms), features_(features) { }

    virtual ~TranslationRule() {} 

    void AddTrgWord(int id) { trg_words_.push_back(id); }
    void AddTrgSym(int id) { trg_syms_.push_back(id); }
    void AddFeature(int id, double feat);
    void AddFeature(const std::string & str, double feat);
    
    virtual bool operator==(const TranslationRule & rhs) const {
        return
            src_str_ == rhs.src_str_ &&
            trg_words_ == rhs.trg_words_ &&
            trg_syms_ == rhs.trg_syms_ &&
            features_ == rhs.features_;
    }
    bool operator!=(const TranslationRule & rhs) const {
        return !(*this == rhs);
    }

    void Print(std::ostream & out) const;

    const std::string & GetSrcStr() const { return src_str_; }
    const std::vector<WordId> & GetTrgWords() const { return trg_words_; }
    const std::vector<WordId> & GetTrgSyms() const { return trg_syms_; }
    const SparseMap & GetFeatures() const { return features_; }
    std::string & GetSrcStr() { return src_str_; }
    std::vector<WordId> & GetTrgWords() { return trg_words_; }
    std::vector<WordId> & GetTrgSyms() { return trg_syms_; }
    SparseMap & GetFeatures() { return features_; }

protected:
    std::string src_str_;
    std::vector<WordId> trg_words_;
    std::vector<WordId> trg_syms_;
    SparseMap features_;

};
inline std::ostream &operator<<( std::ostream &out, const TranslationRule &L ) {
    L.Print(out);
    return out;
}

}

#endif
