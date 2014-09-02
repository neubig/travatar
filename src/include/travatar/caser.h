#ifndef CASER_H__
#define CASER_H__

#include <travatar/sentence.h>
#include <boost/unordered_map.hpp>

namespace travatar {

// A class to perform various casing operation
class Caser {

class HyperGraph;

public:
    Caser() { }
    ~Caser() { }

    WordId ToLower(const std::string & wid);
    WordId ToLower(WordId wid);
    void ToLower(Sentence & sent);
    void ToLower(HyperGraph & graph);

    WordId ToTitle(const std::string & wid);
    WordId ToTitle(WordId wid);
    void ToTitle(Sentence & sent);
    void ToTitle(HyperGraph & graph);

    WordId TrueCase(const std::string & wid);
    WordId TrueCase(WordId wid);
    void TrueCase(Sentence & sent);
    void TrueCase(HyperGraph & graph);

protected:
    boost::unordered_map<std::string, WordId> truecase_map_;

};

}

#endif
