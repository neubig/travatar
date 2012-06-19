#ifndef TRAVATAR_RULE_EXTRACTOR__
#define TRAVATAR_RULE_EXTRACTOR__

#include <boost/shared_ptr.hpp>
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <set>

namespace travatar {

// A virtual class to overload that converts a source parse, a target sentence
// and an alignment into a forest of matched rules
class RuleExtractor {
public:
    // Create the minimal graph of rules
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const = 0;
};

// A rule extractor using forest based rule extraction
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
class ForestExtractor : public RuleExtractor{
public:
    // Extract the very minimal set of rules (no nulls, etc)
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const;

};

}

#endif
