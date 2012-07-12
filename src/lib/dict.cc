#include <travatar/dict.h>
#include <travatar/symbol-set.h>

using namespace travatar;

bool Dict::add_ = true;
SymbolSet<WordId> travatar::Dict::wids_ = SymbolSet<WordId>();

std::string Dict::PrintFeatures(const SparseMap & feats) {
    std::ostringstream oss;
    int sent = 0;
    BOOST_FOREACH(const SparsePair & kv, feats) {
        if(sent++ != 0) oss << ' ';
        oss << Dict::WSym(kv.first) << '=' << kv.second;
    }
    return oss.str();
}
// Get the word ID
SparseMap Dict::ParseFeatures(std::istream & iss) {
    std::string buff;
    SparseMap ret;
    while(iss >> buff) {
        std::vector<std::string> columns;
        boost::algorithm::split(columns, buff, boost::is_any_of("="));
        if(columns.size() != 2) THROW_ERROR("Bad feature string @ " << buff);
        ret.insert(MakePair(Dict::WID(columns[0]), atof(columns[1].c_str())));
    }
    return ret;
}
SparseMap Dict::ParseFeatures(const std::string & str) {
    std::istringstream iss(str);
    return ParseFeatures(iss);
}
