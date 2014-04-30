#ifndef HIERO_RULE_TABLE__
#define HIERO_RULE_TABLE__

#include <set>
#include <list>
#include <map>
#include <sstream>
#include <travatar/dict.h>
#include <travatar/util.h>

#define HIERO_SOURCE 0
#define HIERO_TARGET 1

namespace travatar {

class HieroRule {
public:
	HieroRule() { 
		source_words = std::vector<WordId>();
		target_words = std::vector<WordId>();
		type = -1;
		nt_side_side = -1;
	}

	void AddWord(WordId word, int non_term=0) {
		if (type == HIERO_SOURCE && !non_term && nt_side_side != 1) {
			nt_side_side = -1;
		}
		if (type == HIERO_SOURCE) {
			if (non_term) source_nt_position.insert(source_words.size());
			source_words.push_back(word);
		} else if (type == HIERO_TARGET) {
			if (non_term) target_nt_position.insert(target_words.size());
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

	std::string GetNontermX(int number) const {
		std::ostringstream ss;
		ss << "x" << number << "";
		return ss.str();
	}

	void AddNontermX(int number) {
		if (type == HIERO_SOURCE) {
			if (nt_side_side >= 0) {
				nt_side_side = 1;
			} else {
				nt_side_side = 0;
			}
		}
		WordId id = Dict::WID(GetNontermX(number));
		AddWord(id,1);
	}

	std::string ToString() const {
		std::ostringstream ss;
		for (int i=0; (unsigned)i < source_words.size(); ++i) {
			if (i) ss << " ";
			if (source_nt_position.find(i) == source_nt_position.end()) {
				ss << "\"" << Dict::WSym(source_words[i])<< "\"";
			} else {
				ss << Dict::WSym(source_words[i]) << ":X";
			}
		}
		ss << " @ X";
		ss << " ||| ";
		for (int i=0; (unsigned)i < target_words.size(); ++i) {
			if (i) ss << " ";
			if (target_nt_position.find(i) == target_nt_position.end()) {
				ss << "\""<<Dict::WSym(target_words[i])<<"\"";
			} else {
				ss <<Dict::WSym(target_words[i]) << ":X";
			}
		}
		ss << " @ X";
		return ss.str();
	}

	int GetNumberOfNonterm(int type = -1) const {
		return (source_nt_position.size() + target_nt_position.size()) / 2;
	}

    bool IsRuleBalanced() const {
        return source_nt_position.size() == target_nt_position.size();
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

	int IsNonterminalSideBySide() {
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
	int type;
	int nt_side_side;
	std::set<int> source_nt_position;
	std::set<int> target_nt_position;
};

struct HieroRuleManager {

	static int IsFiltered(HieroRule & rule) {
		int nterm = rule.GetNumberOfNonterm();
		// RULE CONTAINS ALL NON TERMINAL FILTER
		if (rule.GetNumberOfWords(HIERO_TARGET) == nterm || rule.GetNumberOfWords(HIERO_SOURCE) == nterm) {
			return 1;
		}
		if (rule.IsNonterminalSideBySide()) {
			return 1;
		}
		if (!rule.IsRuleBalanced()) {
			THROW_ERROR("The number of NT in source and target is not balance in rule: " + rule.ToString());
		}
		return 0;
	}

	static void AddRule(std::vector<HieroRule> & target, HieroRule & rule) {
		if (!IsFiltered(rule)) {
			target.push_back(rule);
		}
	}

	static std::vector<HieroRule> GlueRules() {
		HieroRule rule = HieroRule();
		rule.SetType(HIERO_SOURCE);
		rule.AddNontermX(1);
		rule.SetType(HIERO_TARGET);
		rule.AddNontermX(1);
		std::vector<HieroRule> ret = std::vector<HieroRule>();
		ret.push_back(rule);
		return ret;
	}
};
}
#endif
