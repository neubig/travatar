#ifndef TRAVATAR_DICT_H__
#define TRAVATAR_DICT_H__

#include <travatar/sentence.h>
#include <travatar/cfg-data.h>
#include <travatar/sparse-map.h>
#include <string>
#include <vector>
#include <iostream>

namespace travatar {

template <class T>
class SymbolSet;

struct Dict {
    // Call Freeze to prevent new IDs from being used
    static void Freeze() {
        add_ = false;
    }

    static const std::string INVALID_SPAN_SYMBOL;

    // Get the word ID
    static WordId WID(const std::string & str);

    // Get the quoted word ID
    static WordId WIDAnnotated(const std::string & str);

    // Get the word symbol
    static const std::string & WSym(WordId id);
    
    // Get the word symbol
    static std::string WSymEscaped(WordId id);

    // Get the
    static std::string WSymAnnotated(WordId id, const Sentence & syms);

    // Print words
    static std::string PrintWords(const Sentence & ids);
    static std::string PrintWords(const CfgDataVector & data);

    // Print annotated words
    static std::string PrintAnnotatedWords(const CfgData & data);
    static std::string PrintAnnotatedVector(const CfgDataVector & data);

    // Feature functions
    static std::string PrintSparseMap(const SparseMap & feats);
    static std::string PrintSparseVector(const SparseVector & feats);
    static SparseMap ParseSparseMap(std::istream & iss);
    static SparseMap ParseSparseMap(const std::string & str);
    static SparseVector ParseSparseVector(std::istream & iss);
    static SparseVector ParseSparseVector(const std::string & str);

    // Get the word ID
    static Sentence ParseWords(const std::string & str);
    static std::vector<Sentence> ParseWordVector(const std::string & str);
    
    // Get the word IDs
    static CfgData ParseAnnotatedWords(const std::string & str);
    static CfgDataVector ParseAnnotatedVector(const std::string & str);

    // Escape a string
    static std::string EscapeString(const std::string & str);
    static std::string EncodeXML(const std::string & str);

private:
    static SymbolSet<WordId> wids_;
    static bool add_;

};

}

#endif
