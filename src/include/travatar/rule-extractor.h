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
    // Create the graph of rules
    // TODO: make HyperGraph const?
    virtual HyperGraph * CreateRuleGraph(
        HyperGraph & src_parse, 
        const Sentence & trg_sent,
        const Alignment & align) const = 0;
};

// A rule extractor using forest based rule extraction
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
class ForestExtractor {
public:

    // Extract graph fragments that correspond to rules
    std::vector<boost::shared_ptr<GraphFragment> > ExtractRules(
            HyperGraph & src_parse, 
            const Sentence & trg_sent,
            const Alignment & align) const;

    // Two functions for calculating frontier nodes according to
    //  "What's in a translation rule?"
    //  Michel Galley, Mark Hopkins, Kevin Knight and Daniel Marcu
    //  NAACL 2004
    // Calculate the spans of each node
    const std::set<int> * CalculateSpan(
        const HyperNode * node,
        const std::vector<std::set<int> > & src_spans,
        std::vector<std::set<int>*> & trg_spans) const;
    // Check whether each node in yield of "node" is in the frontier cluster
    int CalculateFrontier(
                   HyperNode * node,
                   const std::vector<std::set<int> > & src_spans,
                   std::vector<std::set<int>*> & trg_spans,
                   const std::set<int> & complement) const;
};

}

#endif
