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

private:
	std::vector<WordId> source_words;
	std::vector<WordId> target_words;
	int source_non_term;
	int target_non_term;
	int type;
};


class HieroRuleTable {

};




}
#endif