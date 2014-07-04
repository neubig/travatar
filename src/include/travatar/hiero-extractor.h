#ifndef TRAVATAR_HIERO_EXTRACTOR__
#define TRAVATAR_HIERO_EXTRACTOR__

#include <vector>
#include <string>
#include <map>
#include <set>
#include <travatar/sentence.h>
#include <travatar/alignment.h>
#include <travatar/dict.h>
#define HIERO_SRC 0
#define HIERO_TRG 1
#define HIERO_UNDEF -1
namespace travatar {
class HieroRule {
protected:
    Sentence src_words_;
    Sentence trg_words_;
    int type_;
    std::set<int> src_nt_position_;
    std::set<int> trg_nt_position_;
public:
    HieroRule() : src_words_(), 
                  trg_words_(),
                  type_(HIERO_UNDEF),
                  src_nt_position_(),
                  trg_nt_position_() { }

    void AddWord(WordId word, int non_term=0);

    void AddNontermX(int number);

    static bool IsRuleBalanced(HieroRule * rule) { return (rule->src_nt_position_).size() == (rule->trg_nt_position_).size(); }

    static bool IsNTSideBySide(HieroRule * rule) ;

    // Mutator
    void SetSrcSent(Sentence & src) { src_words_ = src; }
    void SetTrgSent(Sentence & trg) { trg_words_ = trg; }
    void SetType(int type) { type_ = type; }

    // Accessor
    int GetNumberOfNT() const { return src_nt_position_.size(); }
    const Sentence & GetSrcSent() const { return src_words_; }
    const Sentence & GetTrgSent() const { return trg_words_; }
    
    // To String
    std::string ToString() const;
};

class HieroExtractor {
    int max_initial_phrase_; 
    int max_terminals_;
public:
    typedef std::pair< std::pair<int,int>, std::pair<int,int> > PhrasePair;
    typedef std::vector< PhrasePair > PhrasePairs;

    HieroExtractor() : max_initial_phrase_(10), max_terminals_(5) { }

    std::vector< std::vector<HieroRule*> > ExtractHieroRule(const Alignment & align, const Sentence & source, const Sentence & target) const;
    
    static void PrintPhrasePairs(
            const PhrasePairs & pairs, 
            const Sentence & source, 
            const Sentence & target);


    static std::string PrintPhrasePair(
            const PhrasePair & pp, 
            const Sentence & source, 
            const Sentence & target);

    PhrasePairs ExtractPhrase(
            const Alignment & align, 
            const Sentence & source, 
            const Sentence & target) const;

    // MUTATOR
    void SetMaxInitialPhrase(const int max_initial_phrase) { max_initial_phrase_ = max_initial_phrase; }
    void SetMaxTerminals(const int max_terminals) { max_terminals_ = max_terminals; }
    
    // ACCESSOR 
    int GetMaxInitialPhrase() const { return max_initial_phrase_; }
    int GetMaxTerminals() const { return max_terminals_; }
private:
    static std::string AppendString(const Sentence & s, const int begin, const int end);
    static int MapMaxKey(const std::map<int,int> & map);
    static int MapMinKey(const std::map<int,int> & map);
    static int QuasiConsecutive(int small, int large, const std::map<int,int> & tp, const std::vector<std::set<int> > & t2s);
    static void ParseRuleWith1Nonterminals(
            const Sentence & sentence, 
            const std::pair<int,int> & pair, 
            const std::pair<int,int> & pair_span, 
            HieroRule * target, 
            const int type);
    
    static void ParseRuleWith2Nonterminals(
            const Sentence & sentence, 
            const std::pair<int,int> & pair1, 
            const std::pair<int,int> & pair2, 
            const std::pair<int,int> & pair_span, 
            HieroRule * target, 
            const int type);
    
    static HieroRule * ParseLexicalTranslationRule(
            const Sentence & source, 
            const Sentence & target, 
            const PhrasePair & pair);

    static HieroRule * ParseUnaryPhraseRule(
            const Sentence & source, 
            const Sentence & target, 
            const PhrasePair & pair, 
            const PhrasePair & pair_span);

    static HieroRule * ParseBinaryPhraseRule(
            const Sentence & source, 
            const Sentence & target, 
            const PhrasePair & pair1, 
            const PhrasePair & pair2, 
            const PhrasePair & pair_span);

    static bool IsRuleValid(HieroRule* rule,int max_terminal);
    static int IsTerritoryOverlapping(const std::pair<int,int> & a, const std::pair<int,int> & b);
    static int IsPhraseOverlapping(const PhrasePair & pair1, const PhrasePair & pair2);

    static int InPhrase(const PhrasePair & p1, const PhrasePair & p2);
};

}

#endif
