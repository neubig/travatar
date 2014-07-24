#include <travatar/word-splitter-compound.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <lm/model.hh>
#include <boost/foreach.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

vector<string> WordSplitterCompound::StringSplit(const std::string & str,
						const std::string & pad) const {
    vector<string> ret;
    ret.push_back(str);
    bool splitted = false;

    lm::ngram::State null_state(lm_->NullContextState()), out_state;
    const lm::ngram::Vocabulary &vocab = lm_->GetVocabulary(); 
    float unigram_score =  lm_->Score(null_state, vocab.Index(str), out_state);
    if (vocab.Index(str) == 0){
      unigram_score = -99; // ensure unigram score of unknown word is extremely low
    }

    // Try to split if unigram score is low or if is an OOV word (and if word satisfies length restriction)
    if ((str.length() > 2*min_char_) && ((unigram_score < logprob_threshold_) || (vocab.Index(str) == 0))){

      float best_score = unigram_score;
      PRINT_DEBUG("Split candidate: " << str << "(" << vocab.Index(str) << ") unigram=" << unigram_score << "\n", 2);

      // Iterate through split position i, try possible fillers,
      // and replace with subwords if their mean probabability is better
      for (unsigned int i=min_char_;i<str.length()-min_char_+1;++i){
	std::string subword1 = str.substr(0,i);

	if (vocab.Index(subword1) != 0){
	  for (unsigned int f=0; f<fillers_.size();++f){
	    int fillchar = fillers_[f].length();

	    if (str.substr(i,fillchar) == fillers_[f]){
	      std::string subword2 = str.substr(i+fillchar);

	      if (vocab.Index(subword2) != 0){
		float score1 = lm_->Score(null_state,vocab.Index(subword1),out_state);
		float score2 = lm_->Score(null_state,vocab.Index(subword2),out_state);
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

    // Read Languaga Model file to get unigram statistics
    // Although we only need unigram, kenlm assumes the file is bigram or above
    lm_ =  new lm::ngram::Model(lm_file.c_str());
  
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

