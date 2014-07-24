#include <travatar/io-util.h>
#include <travatar/global-debug.h>

using namespace std;
using namespace travatar;

// Trim the white space from the front of a stream
void IoUtil::Trim(istream & in, const char* delim) {
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

// Read in a string until we hit a delimiter, and throw an error if we hit a forbidden character
string IoUtil::ReadUntil(istream & in, const char* delim, const char* forbid) {
    char c;
    int i;
    ostringstream oss;
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
