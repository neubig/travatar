#include <cfloat>

#ifdef TRAVATAR_REAL_FLOAT
    typedef float Real;
    #define REAL_MAX FLT_MAX
#else
    typedef double Real;
    #define REAL_MAX DBL_MAX
#endif
