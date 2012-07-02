#ifndef TRAVATAR_DICT_H__
#define TRAVATAR_DICT_H__

#include <string>
#include <vector>
#include <climits>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <travatar/symbol-set.h>
#include <travatar/util.h>

namespace travatar {

typedef int WordId;
typedef std::vector<WordId> Sentence;

struct Dict {
    // Call Freeze to prevent new IDs from being used
    static void Freeze() {
        add_ = false;
    }

    // Get the word ID
    static WordId WID(const std::string & str) {
        return wids_.GetId(str, add_);
    }

    // Get the quoted word ID
    static WordId QuotedWID(const std::string & str) {
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

    // Get the word symbol
    static const std::string & WSym(WordId id) {
        return wids_.GetSymbol(id);
    }

    // Get the
    static std::string WAnnotatedSym(WordId id) {
        std::ostringstream oss;
        if(id < 0)
            oss << "x" << -1+id*-1;
        else
            oss << '"' << wids_.GetSymbol(id) << '"';
        return oss.str();
    }

    // Get the word ID
    static std::vector<WordId> ParseWords(const std::string & str) {
        std::istringstream iss(str);
        std::string buff;
        std::vector<WordId> ret;
        while(iss >> buff)
            ret.push_back(WID(buff));
        return ret;
    }
    
    // Get the word ID
    static std::vector<WordId> ParseQuotedWords(const std::string & str) {
        std::istringstream iss(str);
        std::string buff;
        std::vector<WordId> ret;
        while(iss >> buff)
            ret.push_back(QuotedWID(buff));
        return ret;
    }

    // Get the word ID
    static SparseMap ParseFeatures(const std::string & str) {
        std::istringstream iss(str);
        std::string buff;
        SparseMap ret;
        while(iss >> buff) {
            std::vector<std::string> columns;
            boost::algorithm::split(columns, buff, boost::is_any_of("="));
            if(columns.size() != 2) THROW_ERROR("Bad feature string " << str);
            ret.insert(MakePair(Dict::WID(columns[0]), atof(columns[1].c_str())));
        }
        return ret;
    }

private:
    static SymbolSet<WordId> wids_;
    static bool add_;

};

}

#endif
