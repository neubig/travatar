#ifndef _TRAVATAR_GLOBAL_DEBUG__
#define _TRAVATAR_GLOBAL_DEBUG__

#include <sstream>
#include <stdexcept>
#include <iostream>

namespace travatar {

class GlobalVars {
public:
    static int debug;
    static int trg_factors;
};

}

#define PRINT_DEBUG(msg, lev) do {            \
        if(lev <= GlobalVars::debug)          \
            std::cerr << msg;                 \
        }                                     \
        while (0);

#define THROW_ERROR(msg) do {                   \
    std::ostringstream oss;                     \
    oss << "ERROR: " << msg;                    \
    throw std::runtime_error(oss.str()); }      \
  while (0);

#endif
