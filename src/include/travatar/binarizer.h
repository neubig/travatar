#ifndef BINARIZER_H__
#define BINARIZER_H__

#include <travatar/graph-transformer.h>
#include <travatar/sentence.h>
#include <map>
#include <string>

namespace travatar {

// A parent class for all classes that binarize trees
class Binarizer : public GraphTransformer {

protected:
    std::map<int,int> barred_map_;

public:

    Binarizer() : barred_map_() { }
    static Binarizer * CreateBinarizerFromString(const std::string & bin);

    WordId GetBarredSymbol(WordId sym) const;

};

}

#endif
