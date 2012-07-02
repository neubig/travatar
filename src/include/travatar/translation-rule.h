#ifndef TRANSLATION_RULE_H__
#define TRANSLATION_RULE_H__

#include <string>

namespace travatar {

class TranslationRule {

public:
    TranslationRule(const std::string & src_str = "",
                    const std::vector<int> & trg_words = std::vector<int>(),
                    const std::vector<double> & features = std::vector<double>()) :
        src_str_(src_str), trg_words_(trg_words), features_(features) { }

    void AddTrgWord(int id) { trg_words_.push_back(id); }
    void AddFeature(double feat) { features_.push_back(feat); }

    bool operator==(const TranslationRule & rhs) const {
        return
            src_str_ == rhs.src_str_ &&
            trg_words_ == rhs.trg_words_ &&
            features_ == rhs.features_;
    }
    bool operator!=(const TranslationRule & rhs) const {
        return !(*this == rhs);
    }

    void Print(std::ostream & out) const {
        out << "{\"src\": \""<<src_str_;
        if(trg_words_.size()) {
            out << ", \"trg_words\": [";
            for(int i = 0; i < (int)trg_words_.size(); i++)
                out << trg_words_[i] << ((i == (int)trg_words_.size()-1) ? "]" : ", ");
        }
        if(features_.size()) {
            out << ", \"features\": [";
            for(int i = 0; i < (int)features_.size(); i++)
                out << features_[i] << ((i == (int)features_.size()-1) ? "]" : ", ");
        }
        out << "}";
    }

    const std::string & GetSrcStr() const { return src_str_; }
    const std::vector<WordId> & GetTrgWords() const { return trg_words_; }
    const std::vector<double> & GetFeatures() const { return features_; }
    std::string & GetSrcStr() { return src_str_; }
    std::vector<WordId> & GetTrgWords() { return trg_words_; }
    std::vector<double> & GetFeatures() { return features_; }

protected:
    std::string src_str_;
    std::vector<WordId> trg_words_;
    std::vector<double> features_;

};
inline std::ostream &operator<<( std::ostream &out, const TranslationRule &L ) {
    L.Print(out);
    return out;
}

}

#endif
