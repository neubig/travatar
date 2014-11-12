#ifndef CFG_DATA_H__
#define CFG_DATA_H__

#include <travatar/sentence.h>
#include <iostream>
#include <vector>

#define UNLABELED -1 

namespace travatar {

class CfgData {
public:
    Sentence words;
    WordId label;
    Sentence syms;

    CfgData(const Sentence & _words = Sentence(),
            WordId _label = UNLABELED,
            const Sentence & _syms = Sentence())
        : words(_words), label(_label), syms(_syms) { }
    
    void AppendChild(const CfgData & child);

    bool operator==(const CfgData & rhs) const {
        return words == rhs.words && label == rhs.label && syms == rhs.syms;
    }

    bool operator<(const CfgData & rhs) const {
        if(label != rhs.label) {
            return label < rhs.label;
        } else if (syms != rhs.syms) {
            return syms < rhs.syms;
        } else {
            return words < rhs.words;
        }
    }

    const std::vector<int> GetNontermPositions() const;

    void Print(std::ostream & out) const;

    WordId GetSym(int id) const {
        return (id < (int)syms.size() ? syms[id] : UNLABELED);
    }

    size_t GetSymSize() const { return syms.size(); }

};

inline std::ostream & operator<<(std::ostream & out, const CfgData & data) {
    data.Print(out);
    return out;
}

typedef std::vector<CfgData> CfgDataVector;

}

#endif
