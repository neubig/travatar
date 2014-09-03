#ifndef CASER_H__
#define CASER_H__

#include <travatar/sentence.h>
#include <travatar/graph-transformer.h>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/locale.hpp>
#include <vector>

namespace travatar {

class HyperGraph;

// A class to perform various casing operation
class Caser : public GraphTransformer {

public:

    typedef enum {
        NONE,
        LOW,
        TITLE,
        TRUE
    } CaserType;

    Caser();
    ~Caser();

    static Caser * CreateCaserFromString(const std::string & str);

    std::string ToLower(const std::string & wid) const;
    WordId ToLower(WordId wid) const;
    void ToLower(Sentence & sent) const;
    void ToLower(HyperGraph & graph) const;

    std::string ToTitle(const std::string & wid) const;
    WordId ToTitle(WordId wid) const;
    void ToTitle(Sentence & sent) const;
    void ToTitle(HyperGraph & graph) const;

    std::string TrueCase(const std::string & wid) const;
    WordId TrueCase(WordId wid) const;
    void TrueCase(Sentence & sent) const;
    void TrueCase(HyperGraph & graph) const;

    std::vector<bool> SentenceFirst(const Sentence & sent) const;
    
    void LoadTrueCaseModel(const std::string & file);

    void AddTrueValue(const std::string & str);

    void SetType(CaserType type) { type_ = type; }

    HyperGraph * TransformGraph(const HyperGraph & graph) const;

protected:
    boost::unordered_map<std::string, std::string> truecase_map_;
    std::string loc_name_;
    std::locale loc_;
    CaserType type_;
    boost::unordered_set<WordId> sentence_end_, delayed_sentence_start_;

};

}

#endif
