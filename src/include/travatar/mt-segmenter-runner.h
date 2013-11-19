#ifndef MT_SEGMENTER_RUNNER_H__ 
#define MT_SEGMENTER_RUNNER_H__

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <travatar/sentence.h>

namespace travatar {

class ConfigMTSegmenterRunner;
class EvalMeasure;
class EvalStats;
typedef boost::shared_ptr<EvalStats> EvalStatsPtr;

// A class to segment MT output
// The algorithm is generally based on
//  Evaluating Machine Translation Output with Automatic Sentence Segmentation
//  Evgeny Matusov, Gregor Leusch, Oliver Bender, Hermann Ney
class MTSegmenterRunner {

protected:
    
    // A data structure for memoized recursion
    typedef std::map<std::pair<int,int>, std::pair<std::pair<int,int>, EvalStatsPtr> > MTSegmenterMemo;

    // Use memoized recursion to find the best path from the current to the next
    MTSegmenterMemo::mapped_type GetNextRefSys(
          const std::vector<Sentence> & ref_sents, const Sentence & sys_corpus,
          boost::shared_ptr<EvalMeasure> & eval_measure,
          std::pair<int,int> curr_refsys, MTSegmenterMemo & memo);

public:

    MTSegmenterRunner() : slack_(2.0) { }
    ~MTSegmenterRunner() { }
    
    // Run the model
    void Run(const ConfigMTSegmenterRunner & config);

    // Segment sys_corpus so that eval_measure becomes highest on ref_sents
    // with a maximum segment length of max_len, and store the result in
    // sys_sents
    void SegmentMT(
        const std::vector<Sentence> & ref_sents, const Sentence & sys_corpus,
        boost::shared_ptr<EvalMeasure> & eval_measure,
        std::vector<Sentence> & sys_sents);

protected:
    double slack_;

};

}

#endif

