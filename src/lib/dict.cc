#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/symbol-set.h>
#include <travatar/hyper-graph.h>
// #include <climits>
// #include <cstdlib>

using namespace travatar;
using namespace std;

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
        size_t pos = buff.rfind('=');
        if(pos == string::npos) THROW_ERROR("Bad feature string @ " << buff);
        ret.insert(make_pair(Dict::WID(buff.substr(0, pos)), atof(buff.substr(pos+1).c_str())));
    }
    return ret;
}
SparseMap Dict::ParseFeatures(const std::string & str) {
    std::istringstream iss(str);
    return ParseFeatures(iss);
}

// Get the quoted word ID
WordId Dict::QuotedWID(const std::string & str) {
    // For x0 -> -1, x1 -> -2, etc.
    if(str[0] == 'x') {
        return -1-atoi(str.substr(1).c_str());
    // Otherwise, string must be quoted
    } else if (str[0] == '"' && str.length() > 2 && str[str.length()-1] == '"') {
        return wids_.GetId(str.substr(1,str.length()-2), add_);
    } else {
        THROW_ERROR("Bad quoted string at " << str);
        return INT_MIN;
    }
}

const std::string & Dict::WSym(WordId id) {
    return wids_.GetSymbol(id);
}

WordId Dict::WID(const std::string & str) {
    return wids_.GetId(str, add_);
}

std::string Dict::WSymEscaped(WordId id) {
    std::string ret = wids_.GetSymbol(id);
    boost::replace_all(ret, "\\", "\\\\");
    boost::replace_all(ret, "\"", "\\\"");
    return ret;
}

std::string Dict::WAnnotatedSym(WordId id) {
    std::ostringstream oss;
    if(id < 0)
        oss << "x" << -1+id*-1;
    else
        oss << '"' << wids_.GetSymbol(id) << '"';
    return oss.str();
}

std::string Dict::PrintWords(const std::vector<WordId> & ids) {
    std::ostringstream oss;
    for(int i = 0; i < (int)ids.size(); i++) {
        if(i != 0) oss << ' ';
        oss << WSym(ids[i]);
    }
    return oss.str();
}

std::string Dict::PrintAnnotatedWords(const std::vector<WordId> & ids, const std::vector<WordId> & syms) {
    std::ostringstream oss;
    // Print the symbols
    for(int i = 0; i < (int)ids.size(); i++) {
        if(i != 0) oss << ' ';
        oss << WAnnotatedSym(ids[i]);
    }
    // Print the symbols if they exists
    if(syms.size() > 0) {
        oss << " @";
        BOOST_FOREACH(WordId id, syms)
            oss << ' ' << WSym(id);
    }
    return oss.str();
}


std::vector<WordId> Dict::ParseWords(const std::string & str) {
    std::istringstream iss(str);
    std::string buff;
    std::vector<WordId> ret;
    while(iss >> buff)
        ret.push_back(WID(buff));
    return ret;
}

void Dict::ParseQuotedWords(const std::string & str, std::vector<WordId> & ids, std::vector<WordId> & syms) {
    ids.clear(); syms.clear();
    std::istringstream iss(str);
    std::string buff;
    // Read the words until the end or until '@'
    while(iss >> buff) {
        if(buff == "@")
            break;
        ids.push_back(QuotedWID(buff));
    }
    // Read the remaining as symbols
    while(iss >> buff)
        syms.push_back(WID(buff));
}

std::string Dict::EscapeString(const std::string & str) {
    ostringstream ret;
    for(int i = 0; i < (int)str.length(); i++) {
        if(str[i] == '\\' || str[i] == '"') ret << '\\';
        ret << str[i];
    }
    return ret.str();
}

std::string Dict::EncodeXML(const std::string & data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(1, data[pos]);  break;
        }
    }
    return buffer;
}

const std::string Dict::INVALID_SPAN_SYMBOL = "@X@";
