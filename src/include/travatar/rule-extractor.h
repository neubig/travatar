#ifndef TRAVATAR_RULE_EXTRACTOR__
#define TRAVATAR_RULE_EXTRACTOR__

#include <set>
#include <list>
#include <map>
#include <string>
#include <ostream>
#include <travatar/sentence.h>

namespace travatar {

class HyperGraph;
class HyperNode;
class HyperEdge;
class Alignment;

typedef std::pair< std::pair<int,int>, WordId > LabeledSpan;
typedef std::map< std::pair<int,int>, WordId > LabeledSpans;

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
        const Alignment & align,
        const LabeledSpans * trg_spans = NULL) const;

private:
    // A function to help print rules recursively
    void PrintRuleSurface(
        const HyperNode & node,
        const Sentence & src_sent,
        std::list<HyperEdge*> & remaining_fragments,
        int & tail_num,
        std::ostream & oss) const;
};

}

#endif
