#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/symbol-set.h>
#include <travatar/hyper-graph.h>

using namespace travatar;
using namespace std;
using namespace boost;

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

const std::string & Dict::WSym(WordId id) {
    return wids_.GetSymbol(id);
}

WordId Dict::WID(const std::string & str) {
    return wids_.GetId(str, add_);
}

std::string Dict::WSymEscaped(WordId id) {
    ostringstream oss;
    if(id < 0) {
        oss << id;
    } else {
        std::string ret = wids_.GetSymbol(id);
        boost::replace_all(ret, "\\", "\\\\");
        boost::replace_all(ret, "\"", "\\\"");
        oss << '"' << ret << '"';
    }
    return oss.str();
}

std::string Dict::WSymAnnotated(WordId id, const Sentence & syms) {
    std::ostringstream oss;
    if(id < 0) {
        int loc = -1+id*-1;
        oss << "x" << loc;
        if(loc < (int)syms.size() && syms[loc] != -1)
            oss << Dict::WSym(syms[loc]);
    } else {
        oss << '"' << wids_.GetSymbol(id) << '"';
    }
    return oss.str();
}

std::string Dict::PrintWords(const Sentence & ids) {
    std::ostringstream oss;
    for(int i = 0; i < (int)ids.size(); i++) {
        if(i != 0) oss << ' ';
        oss << WSym(ids[i]);
    }
    return oss.str();
}

std::string Dict::PrintWords(const CfgDataVector & data) {
    std::ostringstream oss;
    for(int i = 0; i < (int)data.size(); i++) {
        if(i != 0) oss << " |COL| ";
        oss << Dict::PrintWords(data[i].words);
    }
    return oss.str();
}

std::string Dict::PrintAnnotatedWords(const CfgData & data) {
    std::ostringstream oss;
    // Print the symbols
    for(int i = 0; i < (int)data.words.size(); i++) {
        if(i != 0) oss << ' ';
        oss << WSymAnnotated(data.words[i], data.syms);
    }
    // Print the head if it exists
    if(data.label != -1)
        oss << " @ " << WSym(data.label);
    return oss.str();
}

std::string Dict::PrintAnnotatedVector(const CfgDataVector & data) {
    std::ostringstream oss;
    for(int i = 0; i < (int)data.size(); i++) {
        if(i != 0) oss << " |COL| ";
        oss << Dict::PrintAnnotatedWords(data[i]);
    }
    return oss.str();
}


Sentence Dict::ParseWords(const std::string & str) {
    std::istringstream iss(str);
    std::string buff;
    Sentence ret;
    while(iss >> buff)
        ret.push_back(WID(buff));
    return ret;
}

std::vector<Sentence> Dict::ParseWordVector(const std::string & str) {
    std::vector<Sentence> ret;
    vector<string> columns;
    algorithm::split_regex(columns, str, regex(" \\|COL\\| "));
    BOOST_FOREACH(const std::string & col, columns) {
        ret.push_back(ParseWords(col));
    }
    return ret;
}

CfgData Dict::ParseAnnotatedWords(const std::string & str) {
    regex term_regex("^\"(.+)\"$");
    regex nonterm_regex("^x(\\d+)(:.+)?$");
    smatch str_match;
    CfgData data;
    std::istringstream iss(str);
    std::string buff;
    // Read all the words in the string
    while(iss >> buff) {
        // The next word is the head symbol
        if(buff == "@") {
            if(!(iss >> buff))
                THROW_ERROR("Missing label in synchronus rule: " <<  str);
            data.label = WID(buff);
            if(iss >> buff)
                THROW_ERROR("Too many labels in synchronus rule: " <<  str);
            break;
        // Read a terminal
        } else if(regex_match(buff, str_match, term_regex)) {
            data.words.push_back(Dict::WID(str_match[1]));
        // Read a non-terminal
        } else if(regex_match(buff, str_match, nonterm_regex)) {
            int id = lexical_cast<int>(str_match[1]);
            data.words.push_back(-1 - id);
            if(str_match[2].length() > 0) {
                if(id >= (int)data.syms.size())
                    data.syms.resize(id+1, -1);
                data.syms[id] = Dict::WID(((string)str_match[2]).substr(1));
            }
        // Everything else is bad
        } else {
            THROW_ERROR("Bad rule string: " << str);
        }
    }
    return data;
}

CfgDataVector Dict::ParseAnnotatedVector(const std::string & str) {
    CfgDataVector ret;
    vector<string> columns;
    algorithm::split_regex(columns, str, regex(" \\|COL\\| "));
    BOOST_FOREACH(const std::string & col, columns) {
        ret.push_back(ParseAnnotatedWords(col));
    }
    return ret;
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
