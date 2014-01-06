#ifndef HIERO_RULE_TABLE__
#define HIERO_RULE_TABLE__

#include <set>
#include <list>
#include <map>
#include <sstream>
#include <travatar/dict.h>
#include <travatar/util.h>

using namespace travatar;
using namespace std;
using namespace boost;

#define HIERO_SOURCE 0
#define HIERO_TARGET 1

namespace travatar{

class HieroRule {
public:
	HieroRule() { 
		source_words = std::vector<WordId>();
		target_words = std::vector<WordId>();
		source_non_term = 0;
		target_non_term = 0;
		type = -1;
		nt_side_side = -1;
	}

	void AddWord(WordId word, int non_term=0) {
		if (type == HIERO_SOURCE && !non_term && nt_side_side != 1) {
			nt_side_side = -1;
		}
		if (type == HIERO_SOURCE) {
			if (non_term) ++source_non_term;
			source_words.push_back(word);
		} else if (type == HIERO_TARGET) {
			if (non_term) ++target_non_term;
			target_words.push_back(word);
		} else {
			THROW_ERROR("Undefined type when adding rule.");
		}
	} 

	void SetSourceSentence(Sentence & source) {
		source_words = source;
	}

	void SetTargetSentence(Sentence & target) {
		target_words = target;
	}

	void SetType(int _type) {
		type = _type;
	}

	string GetNonTermX(int number) const {
		std::ostringstream ss;
		ss << "x" << number << "";
		return ss.str();
	}

	void AddNonTermX(int number) {
		if (type == HIERO_SOURCE) {
			if (nt_side_side >= 0) {
				nt_side_side = 1;
			} else {
				nt_side_side = 0;
			}
		}

		WordId id = Dict::WID(GetNonTermX(number));
		AddWord(id,1);
	}

	string ToString() const {
		std::ostringstream ss;
		for (int i=0; (unsigned)i < source_words.size(); ++i) {
			if (i > 0) ss << " ";
			ss << Dict::WSym(source_words[i]);
		}
		ss << " |||";
		for (int i=0; (unsigned)i < target_words.size(); ++i) {
			ss << " ";
			ss << Dict::WSym(target_words[i]);
		}
		return ss.str();
	}

	int GetNumberOfNonTerm(int type = -1) const {
		return (source_non_term + target_non_term) / 2;
	}

	int GetNumberOfWords(int type = -1) const {
		switch (type) {
			case HIERO_SOURCE:
				return source_words.size();
			case HIERO_TARGET:
				return target_words.size();
			default:
				THROW_ERROR("Undefined type when requesting number of words.");
		}
		return -1;
	}

	int IsNonTerminalSideBySide() {
		return nt_side_side == 1;
	}

	const Sentence & GetSourceSentence() {
		return source_words;
	}

	const Sentence & GetTargetSentence() {
		return target_words;
	}

private:
	Sentence source_words;
	Sentence target_words;
	int source_non_term;
	int target_non_term;
	int type;
	int nt_side_side;
};

struct HieroRuleManager {

	static int IsFiltered(HieroRule & rule) {
		int nterm = rule.GetNumberOfNonTerm();
		// RULE CONTAINS ALL NON TERMINAL FILTER
		if (rule.GetNumberOfWords(HIERO_TARGET) == nterm || rule.GetNumberOfWords(HIERO_SOURCE) == nterm) {
			return 1;
		}
		if (rule.IsNonTerminalSideBySide()) {
			return 1;
		}


		return 0;
	}

	static void AddRule(vector<HieroRule> & target, HieroRule & rule) {
		if (!IsFiltered(rule)) {
			target.push_back(rule);
		}
	}

	static std::vector<HieroRule> GlueRules() {
		HieroRule rule = HieroRule();
		rule.SetType(HIERO_SOURCE);
		rule.AddNonTermX(1);
		rule.SetType(HIERO_TARGET);
		rule.AddNonTermX(1);
		std::vector<HieroRule> ret = std::vector<HieroRule>();
		ret.push_back(rule);
		return ret;
	}
};
}
#endif