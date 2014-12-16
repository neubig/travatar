#ifndef RESCORER_H__ 
#define RESCORER_H__

#include <travatar/sentence.h>
#include <travatar/sparse-map.h>
#include <travatar/real.h>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>

namespace travatar {

class ConfigRescorer;
class EvalMeasure;

class RescorerNbestElement {
public:
    RescorerNbestElement() : score(0.0) { }
    RescorerNbestElement(const Sentence & se, const SparseVector & f, Real sc) :
        sent(se), feat(f), score(sc) { }
    Sentence sent;
    SparseVector feat;
    Real score;
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
    Real mbr_scale_;
    int mbr_hyp_cnt_;
    

};

}

#endif

