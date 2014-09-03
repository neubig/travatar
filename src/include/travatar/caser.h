#ifndef CASER_H__
#define CASER_H__

#include <travatar/sentence.h>
#include <boost/unordered_map.hpp>
#include <boost/locale.hpp>
#include <vector>

namespace travatar {

// A class to perform various casing operation
class Caser {

class HyperGraph;

public:
    Caser();
    ~Caser();

    std::string ToLower(const std::string & wid);
    WordId ToLower(WordId wid);
    void ToLower(Sentence & sent);
    void ToLower(HyperGraph & graph);

    std::string ToTitle(const std::string & wid);
    WordId ToTitle(WordId wid);
    void ToTitle(Sentence & sent);
    void ToTitle(HyperGraph & graph);

    std::string TrueCase(const std::string & wid);
    WordId TrueCase(WordId wid);
    void TrueCase(Sentence & sent);
    void TrueCase(HyperGraph & graph);

    std::vector<bool> SentenceFirst(const Sentence & sent);

    void AddTrueValue(const std::string & str);

protected:
    boost::unordered_map<std::string, std::string> truecase_map_;
    std::string loc_name_;
    std::locale loc_;

};

}

#endif
