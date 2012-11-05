#ifndef TRAVATAR_IO_UTIL__
#define TRAVATAR_IO_UTIL__

#define WHITE_SPACE " \t\n\r"

#include <iostream>
#include <sstream>

namespace travatar {

class IoUtil {
public:
    // Trim the white space from the front of a stream
    static void Trim(std::istream & in, const char* delim);
    static void Trim(std::istream & in, const std::string& delim) {
        IoUtil::Trim(in, delim.c_str());
    }
    
    // Read in a string until we hit a delimiter, and throw an error if we hit a forbidden character
    static std::string ReadUntil(std::istream & in, const char* delim, const char* forbid = "");
    static std::string ReadUntil(std::istream & in, const std::string& delim, const std::string& forbid = "") {
        return IoUtil::ReadUntil(in, delim.c_str(), forbid.c_str());
    }

};

}

#endif
