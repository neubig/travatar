#ifndef TRAVATAR_IO_UTIL__
#define TRAVATAR_IO_UTIL__

#define WHITE_SPACE " \t\n\r"

#include <iostream>
#include <sstream>

namespace travatar {

// Trim the white space from the front of a stream
inline void Trim(std::istream & in, const char* delim) {
    char c;
    int i = 0;
    while(delim[i] != 0 && !in.eof()) {
        c = in.peek();
        for(i = 0; delim[i] != 0; i++) {
            // If they match, remove it and continue
            if(c == delim[i]) {
                in.get();
                break;
            }
        }
    }
}
inline void Trim(std::istream & in, const std::string& delim) {
    Trim(in, delim.c_str());
}

// Read in a string until we hit a delimiter, and throw an error if we hit a forbidden character
// TODO: This probably shouldn't be inline
inline std::string ReadUntil(std::istream & in, const char* delim, const char* forbid = "") {
    char c;
    int i;
    std::ostringstream oss;
    while(in) {
        c = in.peek();
        // If we match a delimiter return
        for(i = 0; delim[i] != 0; i++)
            if(c == delim[i])
                return oss.str();
        // If we match a forbidden character, throw an error
        for(i = 0; forbid[i] != 0; i++)
            if(c == forbid[i])
                THROW_ERROR("Forbidden character " << c);
        // Add the character
        oss << (char)in.get();
    }
    return oss.str();
}
inline std::string ReadUntil(std::istream & in, const std::string& delim, const std::string& forbid = "") {
    return ReadUntil(in, delim.c_str(), forbid.c_str());
}

}

#endif
