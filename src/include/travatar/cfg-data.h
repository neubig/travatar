#ifndef CFG_DATA_H__
#define CFG_DATA_H__

#include <travatar/sentence.h>
#include <iostream>
#include <vector>

namespace travatar {

class CfgData {
public:
    Sentence words;
    WordId label;
    Sentence syms;

    CfgData(const Sentence & _words = Sentence(),
            WordId _label = -1,
            const Sentence & _syms = Sentence())
        : words(_words), label(_label), syms(_syms) { }
    
    void AppendChild(const CfgData & child);

    bool operator==(const CfgData & rhs) const {
        return words == rhs.words && label == rhs.label && syms == rhs.syms;
    }

    const std::vector<int> GetNontermPositions() const;

    void Print(std::ostream & out) const;

    WordId GetSym(int id) const {
        return (id < (int)syms.size() ? syms[id] : -1);
    }

};

inline std::ostream & operator<<(std::ostream & out, const CfgData & data) {
    data.Print(out);
    return out;
}

typedef std::vector<CfgData> CfgDataVector;

}

#endif
