#include <travatar/word-splitter-compound.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

vector<string> WordSplitterCompound::StringSplit(const std::string & str,
						const std::string & pad) const {
    //TODO: stub
    vector<string> ret;
    ret.push_back(str);

    lm::ngram::State null_state(lm_->NullContextState()), out_state;
    const lm::ngram::Vocabulary &vocab = lm_->GetVocabulary(); 
    std::cout << str << " " << lm_->Score(null_state, vocab.Index(str), out_state) << '\n';

    
    return ret;
}

