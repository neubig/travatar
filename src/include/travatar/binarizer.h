#ifndef BINARIZER_H__
#define BINARIZER_H__

#include <travatar/graph-transformer.h>
#include <string>

namespace travatar {

// A parent class for all classes that binarize trees
class Binarizer : public GraphTransformer {

public:
    static Binarizer * CreateBinarizerFromString(const std::string & bin);

};

}

#endif
