#ifndef RESCORER_H__ 
#define RESCORER_H__

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <travatar/eval-measure.h>

namespace travatar {

class ConfigRescorer;

class RescorerNbestElement {
public:
    RescorerNbestElement() : score(0.0) { }
    RescorerNbestElement(const Sentence & se, const SparseMap & f, double sc) :
        sent(se), feat(f), score(sc) { }
    Sentence sent;
    SparseMap feat;
    double score;
};
typedef std::vector<RescorerNbestElement> RescorerNbest;
inline bool operator< ( const RescorerNbestElement& lhs,
                        const RescorerNbestElement& rhs ) {
    if(lhs.score != rhs.score) return lhs.score > rhs.score;
    return lhs.sent < rhs.sent;
}

class RescorerRunner {
public:

    RescorerRunner() : rescore_weights_(false), sent_(0),
                       mbr_scale_(1.0), mbr_hyp_cnt_(0) { }
    ~RescorerRunner() { }
    
    // Read in the entire n-best one by one and rescore
    void Run(const ConfigRescorer & config);

    // Rescore an n-best list
    void Rescore(RescorerNbest & nbest);

    // Print at least the top of the rescored n-best list
    void Print(const RescorerNbest & nbest);

protected:
    SparseMap weights_;
    bool rescore_weights_;
    boost::shared_ptr<std::ofstream> nbest_out_;
    int sent_;
    // For minimum Bayes risk rescoring
    boost::shared_ptr<EvalMeasure> mbr_eval_;
    double mbr_scale_;
    int mbr_hyp_cnt_;
    

};

}

#endif

