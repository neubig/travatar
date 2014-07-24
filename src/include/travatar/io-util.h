#ifndef TRAVATAR_IO_UTIL__
#define TRAVATAR_IO_UTIL__

#define WHITE_SPACE " \t\n\r"

#include <travatar/global-debug.h>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <sstream>
#include <set>

namespace std {

// Output function for pairs
template <class X, class Y>
inline std::ostream& operator << ( std::ostream& out, 
                                   const std::pair< X, boost::shared_ptr<Y> >& rhs )
{
    out << "[" << rhs.first << ", " << *rhs.second << "]";
    return out;
}

// Output function for pairs
template <class X, class Y>
inline std::ostream& operator << ( std::ostream& out, 
                                   const std::pair< X, Y >& rhs )
{
    out << "[" << rhs.first << ", " << rhs.second << "]";
    return out;
}

// Output function for sets
template <class X>
inline std::ostream& operator << ( std::ostream& out, 
                                   const std::set< X >& rhs )
{
    out << "[";
    int val = 0;
    BOOST_FOREACH(const X & x, rhs) {
        if(val++) out << " ";
        out << x;
    }
    out << "]";
    return out;
}

// Input function for pairs
template <class X, class Y>
inline std::istream & operator>> (std::istream & in, std::pair<X,Y>& s) {
    string open, close;
    in >> open >> s.first >> s.second >> close;
    if(open != "<")
        THROW_ERROR("Bad start of pair " << open);
    if(close != "<")
        THROW_ERROR("Bad end of pair " << close);
    return in;
}

}

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
