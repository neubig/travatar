#include <boost/foreach.hpp>
#include <travatar/translation-rule-hiero.h>
#include <travatar/sparse-map.h>
#include <sstream>
#include <travatar/dict.h>

using namespace std;
using namespace travatar;

string TranslationRuleHiero::ToString() {
	std::ostringstream ss;
    ss << Dict::PrintAnnotatedWords(src_data_) << " ||| " << Dict::PrintAnnotatedVector(trg_data_);
	return ss.str();
}
