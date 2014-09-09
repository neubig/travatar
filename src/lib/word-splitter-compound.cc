#include <travatar/word-splitter-compound.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <travatar/lm-composer.h>
#include <lm/model.hh>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;


vector<string> WordSplitterCompound::StringSplit(
                        const std::string & str,
						const std::string & pad) const {
    switch(lm_->GetType()) {
    case lm::ngram::PROBING:
      return StringSplit(str, pad, *(lm::ngram::ProbingModel*)lm_->GetLM());
    case lm::ngram::REST_PROBING:
      return StringSplit(str, pad, *(lm::ngram::RestProbingModel*)lm_->GetLM());
    case lm::ngram::TRIE:
      return StringSplit(str, pad, *(lm::ngram::TrieModel*)lm_->GetLM());
    case lm::ngram::QUANT_TRIE:
      return StringSplit(str, pad, *(lm::ngram::QuantTrieModel*)lm_->GetLM());
    case lm::ngram::ARRAY_TRIE:
      return StringSplit(str, pad, *(lm::ngram::ArrayTrieModel*)lm_->GetLM());
    case lm::ngram::QUANT_ARRAY_TRIE:
      return StringSplit(str, pad, *(lm::ngram::QuantArrayTrieModel*)lm_->GetLM());
    default:
      THROW_ERROR("Unrecognized kenlm model type " << lm_->GetType());
    }
}

template <class LMType>
vector<string> WordSplitterCompound::StringSplit(
                        const std::string & str,
						const std::string & pad,
                        const LMType & my_lm) const {
    vector<string> ret;
    ret.push_back(str);
    bool splitted = false;

    lm::ngram::State null_state(my_lm.NullContextState()), out_state;
    float unigram_score =  my_lm.Score(null_state, my_lm.GetVocabulary().Index(str), out_state);
    if (my_lm.GetVocabulary().Index(str) == 0){
      unigram_score = -99; // ensure unigram score of unknown word is extremely low
    }

    // Try to split if unigram score is low or if is an OOV word (and if word satisfies length restriction)
    if ((str.length() > 2*min_char_) && ((unigram_score < logprob_threshold_) || (my_lm.GetVocabulary().Index(str) == 0))){

      float best_score = unigram_score;
      PRINT_DEBUG("Split candidate: " << str << "(" << my_lm.GetVocabulary().Index(str) << ") unigram=" << unigram_score << "\n", 2);

      // Iterate through split position i, try possible fillers,
      // and replace with subwords if their mean probabability is better
      for (unsigned int i=min_char_;i<str.length()-min_char_+1;++i){
	std::string subword1 = str.substr(0,i);

	if (my_lm.GetVocabulary().Index(subword1) != 0){
	  for (unsigned int f=0; f<fillers_.size();++f){
	    int fillchar = fillers_[f].length();

	    if (str.substr(i,fillchar) == fillers_[f]){
	      std::string subword2 = str.substr(i+fillchar);

	      if (my_lm.GetVocabulary().Index(subword2) != 0){
		float score1 = my_lm.Score(null_state,my_lm.GetVocabulary().Index(subword1),out_state);
		float score2 = my_lm.Score(null_state,my_lm.GetVocabulary().Index(subword2),out_state);
		float mean_score = (score1 + score2) / 2.0;
		PRINT_DEBUG("  " << subword1 << " " << score1 << " + " \
			    << "(" << fillers_[f] << ") +" << subword2 << " " << score2 \
			    << " mean=" << mean_score << "\n",2);
		if (mean_score > best_score){
		  best_score = mean_score;
		  ret.clear();
		  ret.push_back(subword1);
		  ret.push_back(subword2);
		  splitted = true;
		}
	      }
	    }
	  }
	}
      }

      (*stat_).candidates += 1;
      if (splitted)
	(*stat_).splits += 1;

    }

    (*stat_).words += 1;
    return ret;
}


WordSplitterCompound::WordSplitterCompound(const std::string & lm_file, unsigned int min_char, float threshold, const std::string & filler = "") : min_char_(min_char) { 

    // Read Language Model file to get unigram statistics
    // Although we only need unigram, kenlm assumes the file is bigram or above
    lm_ = new LMData(lm_file);
  
    // Setup fillers
    boost::regex delimeter(":");
    boost::sregex_iterator i(filler.begin(),filler.end(),delimeter);
    boost::sregex_iterator j;
    int pos = 0;
    for (;i !=j; ++i){
      if (i->position() != pos)
	fillers_.push_back(filler.substr(pos,i->position()-pos));
      pos = i->position()+1;
    }
    if (pos != (int)filler.size())
      fillers_.push_back(filler.substr(pos));
    fillers_.push_back(""); // always include "" (no filler). 


    // Start recording statistics
    stat_ = new SplitStatistics();

    // Note: we rely on user input for now, but it may be possible
    // to automatically determine threshold if there's a good formula, e.g.
    // logprob_threshold_ = log10(1000./(float)lm_->GetVocabulary().Bound());
    logprob_threshold_ = log10(threshold);

}


WordSplitterCompound::~WordSplitterCompound() {

  PRINT_DEBUG("WordSplitterCompound(): Threshold=" << pow(10,logprob_threshold_) \
	      << " MinChar=" << min_char_ << "\n"			\
	      << "   Processed "  << stat_->words << " words, "		\
	      << stat_->candidates << " split candidates, "		\
	      << stat_->splits << " splitted\n",1);

  if (lm_) delete lm_;
  delete stat_;
}

