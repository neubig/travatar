#ifndef TRAVATAR_DICT_H__
#define TRAVATAR_DICT_H__

#include <string>
#include <vector>
#include <iostream>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>

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
    static WordId QuotedWID(const std::string & str);

    // Get the word symbol
    static const std::string & WSym(WordId id);
    
    // Get the word symbol
    static std::string WSymEscaped(WordId id);

    // Get the
    static std::string WAnnotatedSym(WordId id);

    // Print words
    static std::string PrintWords(const Sentence & ids);

    // Print annotated words
    static std::string PrintAnnotatedWords(const Sentence & ids, const Sentence & syms);

    // Feature functions
    static std::string PrintFeatures(const SparseMap & feats);
    static SparseMap ParseFeatures(std::istream & iss);
    static SparseMap ParseFeatures(const std::string & str);

    // Get the word ID
    static Sentence ParseWords(const std::string & str);
    
    // Get the word ID
    static void ParseQuotedWords(const std::string & str, Sentence & ids, Sentence & syms);

    // Escape a string
    static std::string EscapeString(const std::string & str);
    static std::string EncodeXML(const std::string & str);

private:
    static SymbolSet<WordId> wids_;
    static bool add_;

};

}

#endif
