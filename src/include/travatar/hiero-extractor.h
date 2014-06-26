#ifndef TRAVATAR_HIERO_EXTRACTOR__
#define TRAVATAR_HIERO_EXTRACTOR__

#include <vector>
#include <string>
#include <map>
#include <set>
#include <travatar/sentence.h>

namespace travatar {

class HieroRule;
class Alignment;
typedef std::pair< std::pair<int,int>, std::pair<int,int> > PhrasePair;
typedef std::vector< PhrasePair > PhrasePairs;

class HieroExtractor {
    int max_initial_phrase_; 
    int max_terminals_;
public:
    HieroExtractor() : max_initial_phrase_(10), max_terminals_(5) { }

    std::vector<std::vector<HieroRule> > ExtractHieroRule(const Alignment & align, const Sentence & source, const Sentence & target) const;
    std::string PrintPhrasePair(const PhrasePair & pp, const Sentence & source, const Sentence & target) const;
    PhrasePairs ExtractPhrase(const Alignment & align, const Sentence & source, const Sentence & target) const;

    // MUTATOR
    void SetMaxInitialPhrase(const int max_initial_phrase) { max_initial_phrase_ = max_initial_phrase; }
    void SetMaxTerminals(const int max_terminals) {max_terminals_ = max_terminals; }
    
    // ACCESSOR 
    int GetMaxInitialPhrase() const { return max_initial_phrase_; }
    int GetMaxTerminals() const { return max_terminals_; }
private:
    std::string AppendString(const Sentence & s, const int begin, const int end) const;
    
    void PrintPhrasePairs(const PhrasePairs & pairs, const Sentence & source, const Sentence & target);

    int MapMaxKey(const std::map<int,int> & map) const;
    int MapMinKey(const std::map<int,int> & map) const;
    int QuasiConsecutive(int small, int large, const std::map<int,int> & tp, const std::vector<std::set<int> > & t2s) const;
    int IsTerritoryOverlapping(const std::pair<int,int> & a, const std::pair<int,int> & b) const;
    int IsPhraseOverlapping(const PhrasePair & pair1, const PhrasePair & pair2) const;

    void ParseRuleWith2Nonterminals(const Sentence & sentence, const std::pair<int,int> & pair1, const std::pair<int,int> & pair2, 
                                    const std::pair<int,int> & pair_span, HieroRule & target, const int type) const;

    HieroRule ParseBinaryPhraseRule(const Sentence & source, const Sentence & target, const PhrasePair & pair1, 
                                    const PhrasePair & pair2, const PhrasePair & pair_span) const;

    void ParseRuleWith1Nonterminals(const Sentence & sentence, const std::pair<int,int> & pair, 
                                    const std::pair<int,int> & pair_span, HieroRule & target, const int type) const;

    HieroRule ParseUnaryPhraseRule(const Sentence & source, const Sentence & target, 
                                    const PhrasePair & pair, const PhrasePair & pair_span) const;

    HieroRule ParsePhraseTranslationRule(const Sentence & source, const Sentence & target, 
                                        const PhrasePair & pair) const;

    // Check whether a rule follows the specified constraints
    bool CheckRule(const HieroRule & rule) const;

    int InPhrase(const PhrasePair & p1, const PhrasePair & p2) const;
};

}

#endif
