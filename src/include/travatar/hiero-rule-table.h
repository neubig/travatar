#ifndef HIERO_RULE_TABLE__
#define HIERO_RULE_TABLE__

#include <set>
#include <list>
#include <map>
#include <sstream>
#include <travatar/dict.h>
#include <travatar/util.h>

#define HIERO_SRC 0
#define HIERO_TRG 1

namespace travatar {

class HieroRule {
public:
	HieroRule() { 
		src_words = std::vector<WordId>();
		trg_words = std::vector<WordId>();
		type = -1;
		nt_side_side = -1;
	}

	void AddWord(WordId word, int non_term=0) {
		if (type == HIERO_SRC && !non_term && nt_side_side != 1) {
			nt_side_side = -1;
		}
		if (type == HIERO_SRC) {
			if (non_term) src_nt_position.insert(src_words.size());
			src_words.push_back(word);
		} else if (type == HIERO_TRG) {
			if (non_term) trg_nt_position.insert(trg_words.size());
			trg_words.push_back(word);
		} else {
			THROW_ERROR("Undefined type when adding rule.");
		}
	} 

	void SetSrcSent(Sentence & src) { src_words = src; }
	void SetTrgSent(Sentence & trg) { trg_words = trg; }
	void SetType(int _type) { type = _type; }

	void AddNontermX(int number) {
		if (type == HIERO_SRC) {
			if (nt_side_side >= 0) {
				nt_side_side = 1;
			} else {
				nt_side_side = 0;
			}
		}
		AddWord(-1-number,1);
	}

	std::string ToString() const {
		std::ostringstream ss;
		for (int i=0; (unsigned)i < src_words.size(); ++i) {
			if (i) ss << " ";
			if (src_nt_position.find(i) == src_nt_position.end()) {
				ss << "\"" << Dict::WSym(src_words[i])<< "\"";
			} else {
				ss << "x" << -1-src_words[i] << ":X";
			}
		}
		ss << " @ X";
		ss << " ||| ";
		for (int i=0; (unsigned)i < trg_words.size(); ++i) {
			if (i) ss << " ";
			if (trg_nt_position.find(i) == trg_nt_position.end()) {
				ss << "\""<<Dict::WSym(trg_words[i])<<"\"";
			} else {
				ss << "x" << -1-trg_words[i] << ":X";
			}
		}
		ss << " @ X";
		return ss.str();
	}

	int GetNumberOfNonterm(int type = -1) const {
		return (src_nt_position.size() + trg_nt_position.size()) / 2;
	}

    bool IsRuleBalanced() const {
        return src_nt_position.size() == trg_nt_position.size();
    }

	int GetNumberOfWords(int type = -1) const {
		switch (type) {
			case HIERO_SRC:
				return src_words.size();
			case HIERO_TRG:
				return trg_words.size();
			default:
				THROW_ERROR("Undefined type when requesting number of words.");
		}
		return -1;
	}

	int IsNonterminalSideBySide() {
		return nt_side_side == 1;
	}

	const Sentence & GetSrcSent() const { return src_words; }
    const Sentence & GetTrgSent() const { return trg_words; }

private:
	Sentence src_words;
	Sentence trg_words;
	int type;
	int nt_side_side;
	std::set<int> src_nt_position;
	std::set<int> trg_nt_position;
};

struct HieroRuleManager {

	static int IsFiltered(HieroRule & rule) {
		int nterm = rule.GetNumberOfNonterm();
		// RULE CONTAINS ALL NON TERMINAL FILTER
		if (rule.GetNumberOfWords(HIERO_TRG) == nterm || rule.GetNumberOfWords(HIERO_SRC) == nterm) {
			return 1;
		}
		if (rule.IsNonterminalSideBySide()) {
			return 1;
		}
		if (!rule.IsRuleBalanced()) {
			THROW_ERROR("The number of NT in src and trg is not balance in rule: " + rule.ToString());
		}
		return 0;
	}

	static void AddRule(std::vector<HieroRule> & trg, HieroRule & rule) {
		if (!IsFiltered(rule)) {
			trg.push_back(rule);
		}
	}

	static std::vector<HieroRule> GlueRules() {
		HieroRule rule = HieroRule();
		rule.SetType(HIERO_SRC);
		rule.AddNontermX(1);
		rule.SetType(HIERO_TRG);
		rule.AddNontermX(1);
		std::vector<HieroRule> ret = std::vector<HieroRule>();
		ret.push_back(rule);
		return ret;
	}
};
}
#endif
