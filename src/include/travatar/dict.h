#ifndef TRAVATAR_DICT_H__
#define TRAVATAR_DICT_H__

#include <string>
#include <travatar/symbol-set.h>

namespace travatar {

typedef int WordId;

struct Dict {
    // Call Freeze to prevent new IDs from being used
    static void Freeze() {
        add_ = false;
    }

    // Get the word ID
    static WordId WID(const std::string & str) {
        return wids_.GetId(str, add_);
    }

    // Get the word symbol
    static const std::string & WSym(WordId id) {
        return wids_.GetSymbol(id);
    }

    // Get the word ID
    static std::vector<WordId> ParseWords(const std::string & str) {
        std::istringstream iss(str);
        std::string buff;
        std::vector<WordId> ret;
        while(iss >> buff)
            ret.push_back(WID(buff));
        return ret;
    }

private:
    static SymbolSet<WordId> wids_;
    static bool add_;

};

}

#endif
