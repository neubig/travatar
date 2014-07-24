#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/symbol-set.h>
#include <travatar/hyper-graph.h>

using namespace travatar;
using namespace std;
using namespace boost;

bool Dict::add_ = true;
SymbolSet<WordId> travatar::Dict::wids_ = SymbolSet<WordId>();

std::string Dict::PrintSparseMap(const SparseMap & feats) {
    std::ostringstream oss;
    int sent = 0;
    BOOST_FOREACH(const SparsePair & kv, feats) {
        if(sent++ != 0) oss << ' ';
        oss << Dict::WSym(kv.first) << '=' << kv.second;
    }
    return oss.str();
}
std::string Dict::PrintSparseVector(const SparseVector & feats) {
    std::ostringstream oss;
    int sent = 0;
    BOOST_FOREACH(const SparsePair & kv, feats.GetImpl()) {
        if(sent++ != 0) oss << ' ';
        oss << Dict::WSym(kv.first) << '=' << kv.second;
    }
    return oss.str();
}
// Get the word ID
SparseMap Dict::ParseSparseMap(std::istream & iss) {
    std::string buff;
    SparseMap ret;
    while(iss >> buff) {
        size_t pos = buff.rfind('=');
        if(pos == string::npos) THROW_ERROR("Bad feature string @ " << buff);
        ret.insert(make_pair(Dict::WID(buff.substr(0, pos)), atof(buff.substr(pos+1).c_str())));
    }
    return ret;
}
SparseMap Dict::ParseSparseMap(const std::string & str) {
    std::istringstream iss(str);
    return ParseSparseMap(iss);
}
SparseVector Dict::ParseSparseVector(std::istream & iss) {
    std::string buff;
    vector<SparsePair> ret;
    while(iss >> buff) {
        size_t pos = buff.rfind('=');
        if(pos == string::npos) THROW_ERROR("Bad feature string @ " << buff);
        ret.push_back(make_pair(Dict::WID(buff.substr(0, pos)), atof(buff.substr(pos+1).c_str())));
    }
    return SparseVector(ret);
}
SparseVector Dict::ParseSparseVector(const std::string & str) {
    std::istringstream iss(str);
    return ParseSparseVector(iss);
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
        oss << '"' << EscapeQuotes(ret) << '"';
    }
    return oss.str();
}

std::string Dict::WSymAnnotated(WordId id, const Sentence & syms) {
    std::ostringstream oss;
    if(id < 0) {
        int loc = -1+id*-1;
        oss << "x" << loc;
        if(loc < (int)syms.size() && syms[loc] != -1)
            oss << ":" << Dict::WSym(syms[loc]);
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
    vector<string> columns = Tokenize(str, " |COL| ");
    BOOST_FOREACH(const std::string & col, columns) {
        ret.push_back(ParseWords(col));
    }
    return ret;
}

CfgData Dict::ParseAnnotatedWords(const std::string & str) {
    CfgData data;
    std::istringstream iss(str);
    std::string buff;
    // Read all the words in the string
    while(iss >> buff) {
        int bs = buff.size();
        // The next word is the head symbol
        if(buff == "@") {
            if(!(iss >> buff))
                THROW_ERROR("Missing label in synchronus rule: " <<  str);
            data.label = WID(buff);
            if(iss >> buff)
                THROW_ERROR("Too many labels in synchronus rule: " <<  str);
            break;
        // Read a terminal
        } else if (bs > 2 && buff[0] == '"' && buff[bs-1] == '"') {
            data.words.push_back(Dict::WID(buff.substr(1, bs-2)));
        // Read a non-terminal
        } else if(buff[0] == 'x') {
            int end;
            for(end = 1; end < bs && buff[end] >= '0' && buff[end] <= '9'; end++);
            if(end == 1) THROW_ERROR("Bad rule string: " << str);
            int id = lexical_cast<int>(buff.substr(1, end-1));
            data.words.push_back(-1 - id);
            if(end != bs) {
                if(buff[end] != ':') THROW_ERROR("Bad rule string: " << str);
                if(id >= (int)data.syms.size())
                    data.syms.resize(id+1, -1);
                data.syms[id] = Dict::WID(buff.substr(end+1));
            }
        // Everything else is bad
        } else {
            THROW_ERROR("Bad rule string: " << str);
        }
    }
    if(data.words.capacity() > data.words.size()) { std::vector<WordId>(data.words).swap(data.words); }
    if(data.syms.capacity() > data.syms.size()) { std::vector<WordId>(data.syms).swap(data.syms); }
    return data;
}

CfgDataVector Dict::ParseAnnotatedVector(const std::string & str) {
    vector<string> columns = Tokenize(str, " |COL| ");
    CfgDataVector ret(columns.size());
    for(size_t i = 0; i < columns.size(); i++) {
        ret[i] = ParseAnnotatedWords(columns[i]);
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
