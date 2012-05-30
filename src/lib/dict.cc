#include <travatar/dict.h>
#include <travatar/symbol-set.h>

using namespace travatar;

bool Dict::add_ = true;
SymbolSet<WordId> travatar::Dict::wids_ = SymbolSet<WordId>();
