#ifndef TRAVATAR_RULE_EXTRACTOR__
#define TRAVATAR_RULE_EXTRACTOR__

#include <set>
#include <list>
#include <map>
#include <travatar/sentence.h>
#include <travatar/hiero-rule-table.h>

namespace travatar {

class HyperGraph;
class HyperNode;
class HyperEdge;
class Alignment;

typedef std::pair< std::pair<int,int>, WordId > LabeledSpan;
typedef std::map< std::pair<int,int>, WordId > LabeledSpans;
typedef std::pair< std::pair<int,int>, std::pair<int,int> > PhrasePair;
typedef std::vector< PhrasePair > PhrasePairs;

// A virtual class to overload that converts a source parse, a target sentence
// and an alignment into a forest of matched rules
class RuleExtractor {
public:

    // Create the minimal graph of rules
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const = 0;

    // Take an edge that represents a rule and the corresponding target sentence
    // and return a string version of the rule
    std::string RuleToString(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent,
        const LabeledSpans * trg_spans = NULL) const;

private:
    // A function to help print rules recursively
    void PrintRuleSurface(const HyperNode & node,
                          const Sentence & src_sent,
                          std::list<HyperEdge*> & remaining_fragments,
                          int & tail_num,
                          std::ostream & oss) const;

};

// A rule extractor using forest based rule extraction
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
class ForestExtractor : public RuleExtractor{
public:

    ForestExtractor() : max_attach_(1), max_nonterm_(5) { }

    // Extract the very minimal set of rules (no nulls, etc)
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const;

    // Attach null-aligned target words to the very top node
    // that they can be attached to
    HyperGraph * AttachNullsTop(const HyperGraph & rule_graph,
                                const Alignment & align,
                                int trg_len);

    // Attach null-aligned target words to all possible nodes that
    // they can be attached to
    HyperGraph * AttachNullsExhaustive(const HyperGraph & rule_graph,
                                       const Alignment & align,
                                       int trg_len);

    // For expanding all nulls exhaustively
    typedef std::vector<std::pair<std::set<int>, HyperNode*> > SpanNodeVector;

    // Used memoized recursion to get the expanded nodes for each old node
    const SpanNodeVector & GetExpandedNodes(
                            const std::vector<bool> & nulls,
                            const HyperNode & old_node,
                            std::vector<SpanNodeVector> & expanded,
                            int my_attach);

    SpanNodeVector ExpandNode(
                const std::vector<bool> & nulls,
                const HyperNode & old_node,
                int my_attach) const;

    void SetMaxAttach(int max_attach) { max_attach_ = max_attach; }
    void SetMaxNonterm(int max_nonterm) { max_nonterm_ = max_nonterm; }

protected:

    int max_attach_;
    int max_nonterm_;

    void AttachNullsTop(std::vector<bool> & nulls,
                        HyperNode & node);
    void AttachNullsExhaustive(std::vector<bool> & nulls,
                               HyperNode & node);

};


class HieroExtractor {
    int max_initial_phrase_len_;
public:
    HieroExtractor() : max_initial_phrase_len_(10) { }

    void SetMaxInitalPhrase(int max_initial_phrase_len) { max_initial_phrase_len_ = max_initial_phrase_len; }

    std::vector<HieroRule> ExtractHieroRule(const Alignment & align, const Sentence & source, const Sentence & target);

    // ACCESSOR 
    int GetMaxInitialPhrase() { return max_initial_phrase_len_; }
private:
    PhrasePairs ExtractPhrase(const Alignment & align, const Sentence & source, const Sentence & target);
    std::string AppendString(const Sentence & s, int begin, int end);
    std::string PrintPhrasePair(const PhrasePair & pp, const Sentence & source, const Sentence & target);
    void PrintPhrasePairs(const PhrasePairs & pairs, const Sentence & source, const Sentence & target);

    int MapMaxKey(const std::map<int,int> & map);
    int MapMinKey(const std::map<int,int> & map);
    int QuasiConsecutive(int small, int large, const std::map<int,int> & tp, const std::vector<set<int> > & t2s);
    int IsTerritoryOverlapping(const pair<int,int> & a, const pair<int,int> & b);
    int IsPhraseOverlapping(const PhrasePair & pair1, const PhrasePair & pair2);

    void ParseRuleWith2NonTerminals(const Sentence & sentence, const std::pair<int,int> & pair1, const std::pair<int,int> & pair2, 
                                    const std::pair<int,int> & pair_span, HieroRule & target, int type);

    HieroRule ParseBinaryPhraseRule(const Sentence & source, const Sentence & target, const PhrasePair & pair1, 
                                    const PhrasePair & pair2, const PhrasePair & pair_span);

    void ParseRuleWith1NonTerminals(const Sentence & sentence, const std::pair<int,int> & pair, 
                                    const std::pair<int,int> & pair_span, HieroRule & target, int type);

    HieroRule ParseUnaryPhraseRule(const Sentence & source, const Sentence & target, 
                                    const PhrasePair & pair, const PhrasePair & pair_span);

    HieroRule ParsePhraseTranslationRule(const Sentence & source, const Sentence & target, 
                                        const PhrasePair & pair);

    int InPhrase(const PhrasePair & p1, const PhrasePair & p2);

};


}

#endif
