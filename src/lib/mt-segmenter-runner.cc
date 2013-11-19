#include <iostream>
#include <fstream>
#include <algorithm>
#include <travatar/util.h>
#include <travatar/mt-segmenter-runner.h>
#include <travatar/config-mt-segmenter-runner.h>
#include <travatar/dict.h>
#include <travatar/eval-measure-bleu.h>
#include <travatar/eval-measure-ribes.h>
#include <travatar/eval-measure-ter.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

// Use memoized recursion to find the best path from the current to the next
MTSegmenterRunner::MTSegmenterMemo::mapped_type MTSegmenterRunner::GetNextRefSys(
      const vector<Sentence> & ref_sents, const Sentence & sys_corpus,
      shared_ptr<EvalMeasure> & eval_measure,
      pair<int,int> curr_refsys, MTSegmenterMemo & memo) {
    // Find if we have an answer for the current value and return it if so
    MTSegmenterMemo::iterator it = memo.find(curr_refsys);
    if(it != memo.end()) return it->second;
    Sentence curr_sentence;
    // Special treatment of the last sentence, only look at ones that go
    // all the way to the end
    int max_len = 3 * ref_sents[curr_refsys.first].size();
    bool is_last = (curr_refsys.first == (int)ref_sents.size()-1);
    int first = (is_last ? (int)sys_corpus.size() : curr_refsys.second);
    int last  = min((int)sys_corpus.size(), curr_refsys.second + max_len);
    // Add all sents up to the candidate span
    for(int i = curr_refsys.second; i < first; i++)
        curr_sentence.push_back(sys_corpus[i]);
    double best_score = -DBL_MAX;
    for(int i = first; i <= last; i++) {
        pair<int,int> next_refsys(curr_refsys.first+1, i);
        EvalStatsPtr eval, next_eval;
        if(!is_last) { 
            // Find the next path
            const MTSegmenterMemo::mapped_type & next_val = GetNextRefSys(
                        ref_sents, sys_corpus, eval_measure, next_refsys, memo);
            // If the value is null, then we are not in the acceptable beam
            if(next_val.second.get() == NULL)
                continue;
            next_eval = next_val.second;
        }
        eval = eval_measure->CalculateCachedStats(
              ref_sents[curr_refsys.first], curr_sentence,
              curr_refsys.first, curr_refsys.second*(sys_corpus.size()+1)+i);
        if(next_eval.get() != NULL) eval->PlusEquals(*next_eval);
        double my_score = eval->ConvertToScore();
        if(my_score > best_score) {
            memo[curr_refsys] = MTSegmenterMemo::mapped_type(next_refsys, eval);
            best_score = my_score;
        }
        if(i != last) curr_sentence.push_back(sys_corpus[i]);
    }
    // If we have not found any answer, add a dummy answer
    if(best_score == -DBL_MAX)
        memo[curr_refsys] = 
            MTSegmenterMemo::mapped_type(make_pair(-1,-1), EvalStatsPtr());
    else
        PRINT_DEBUG("Processed ref=" << curr_refsys.first << " sent, sys=" << curr_refsys.second << " word, score=" << best_score << endl, 2);
    return memo[curr_refsys];
}


// Segment sys_corpus so that eval_measure becomes highest on ref_sents
// with a maximum segment length of max_len, and store the result in
// sys_sents
void MTSegmenterRunner::SegmentMT(
        const vector<Sentence> & ref_sents, const Sentence & sys_corpus,
        shared_ptr<EvalMeasure> & eval_measure, int max_len,
        vector<Sentence> & sys_sents) {
    MTSegmenterMemo memo;
    pair<int,int> curr_refsys(0,0);
    while(curr_refsys.first != (int)ref_sents.size()) {
        MTSegmenterMemo::mapped_type next_refsys = GetNextRefSys(
            ref_sents, sys_corpus, eval_measure,
            curr_refsys, memo);
        sys_sents.push_back(Sentence());
        for(int i = curr_refsys.second; i < next_refsys.first.second; i++)
            sys_sents.rbegin()->push_back(sys_corpus[i]);
        curr_refsys = next_refsys.first;
    }
}

// Run the model
void MTSegmenterRunner::Run(const ConfigMTSegmenterRunner & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Load the reference
    ifstream refin(config.GetString("ref").c_str());
    if(!refin)
        THROW_ERROR("Could not open reference: " << config.GetString("ref"));
    vector<Sentence> ref_sents;
    string line;
    int max_ref_size = 0;
    while(getline(refin, line)) {
        ref_sents.push_back(Dict::ParseWords(line));
        max_ref_size = max((int)ref_sents.rbegin()->size(), max_ref_size);
    }
    
    // Load the evaluation measure
    shared_ptr<EvalMeasure> eval_measure(
        EvalMeasure::CreateMeasureFromString(config.GetString("eval")));

    // Load the file to be split
    const string & filename = config.GetMainArg(0);
    ifstream sysin(filename.c_str());
    Sentence sys_corpus;
    while(getline(sysin, line))
        BOOST_FOREACH(WordId wid, Dict::ParseWords(line))
            sys_corpus.push_back(wid);

    // Call the function to split the corpus
    vector<Sentence> sys_sents;
    SegmentMT(ref_sents, sys_corpus, eval_measure, max_ref_size*2,
              sys_sents);

    // Print out the segmentation
    BOOST_FOREACH(const Sentence & sent, sys_sents)
        cout << Dict::PrintWords(sent) << endl;

}
