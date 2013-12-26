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
	}

	void AddWord(WordId & word, int non_term=0) {
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

	void SetType(int _type) {
		type = _type;
	}

	string GetNonTermX(int number) {
		std::ostringstream ss;
		ss << "[X" << number << "]";
		return ss.str();
	}

	void AddNonTermX(int number) {
		WordId id = Dict::WID(GetNonTermX(number));
		AddWord(id,1);
	}

	string ToString() {
		std::ostringstream ss;
		ss << "<";
		for (int i=0; (unsigned)i < source_words.size(); ++i) {
			if (i > 0) ss << " ";
			ss << Dict::WSym(source_words[i]);
		}
		ss << " |||";
		for (int i=0; (unsigned)i < target_words.size(); ++i) {
			ss << " ";
			ss << Dict::WSym(target_words[i]);
		}
		ss << ">";
		return ss.str();
	}

	int GetNumberOfNonTerm(int type = -1) {
		return (source_non_term + target_non_term) / 2;
	}

	int GetNumberOfWords(int type = -1) {
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

	Sentence GetSourceSentence() {
		return source_words;
	}

	Sentence GetTargetSentence() {
		return target_words;
	}

private:
	Sentence source_words;
	Sentence target_words;
	int source_non_term;
	int target_non_term;
	int type;
};

static std::set<long long int> rule_set;

struct HieroRuleManager {
	static int IsFiltered(HieroRule rule) {
		int nterm = rule.GetNumberOfNonTerm();
		// RULE CONTAINS ALL NON TERMINAL FILTER
		if (rule.GetNumberOfWords(HIERO_TARGET) == nterm || rule.GetNumberOfWords(HIERO_SOURCE) == nterm) {
			return 1;
		}

		// DUPLICATION-FILTER
		// who cares that rules may have length of 7 from source and they may duplicate?
		// maybe there is some, but yes, don't be so defensive.
		if (rule.GetNumberOfWords(HIERO_SOURCE) > 7) {
			// Get the HashValue [we do primitive hash value implementation
			// with assumption of perfect hashing]
			Sentence source = rule.GetSourceSentence();
			long long int value = 0;
			long long int multiplier = 31; // some fancy prime number
			for (int i=0; i < (int)source.size(); ++i) {
				value += (long long int) source[i] * multiplier;
				multiplier *= 31;
			}

			// check our set whether rule is in set or not
			if (rule_set.find(value) == rule_set.end()) {
				rule_set.insert(value);
			} else {
				return 1; // rule in set, filter it!
			}
		}
		return 0;
	}

	static void AddRule(vector<HieroRule> & target, HieroRule & rule) {
		if (!IsFiltered(rule)) {
			target.push_back(rule);
		}
	}
};
}
#endif