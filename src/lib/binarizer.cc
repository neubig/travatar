#include <travatar/binarizer.h>
#include <travatar/binarizer-directional.h>
#include <travatar/binarizer-cky.h>
#include <travatar/global-debug.h>

using namespace travatar;
using namespace std;
using namespace boost;

Binarizer * Binarizer::CreateBinarizerFromString(const std::string & bin) {
    if(bin == "left") {
        return new BinarizerDirectional(BinarizerDirectional::BINARIZE_LEFT);
    } else if(bin == "leftrp") {
        return new BinarizerDirectional(BinarizerDirectional::BINARIZE_LEFT, true);
    } else if(bin == "right") {
        return new BinarizerDirectional(BinarizerDirectional::BINARIZE_RIGHT);
    } else if(bin == "rightrp") {
        return new BinarizerDirectional(BinarizerDirectional::BINARIZE_RIGHT, true);
    } else if(bin == "cky") {
        return new BinarizerCKY;
    } else if(bin != "none") {
        THROW_ERROR("Invalid binarizer_ type " << bin);
    }
    return NULL;
}

WordId Binarizer::GetBarredSymbol(WordId sym) const {
    map<int,int>::const_iterator it = barred_map_.find(sym);
    if(it != barred_map_.end()) return it->second;
    WordId ret = Dict::WID(Dict::WSym(sym)+"'");
    const_cast<map<int,int>& >(barred_map_).insert(make_pair(sym, ret));
    return ret;
}
