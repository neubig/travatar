#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([travatar], [0.1.0], [neubig@gmail.com])
AC_CONFIG_SRCDIR(src/lib/travatar.cc)
AC_CONFIG_HEADERS([src/include/travatar/config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/liblbfgs/Makefile
    src/marisa/Makefile
    src/tercpp/Makefile
    src/kenlm/Makefile
    src/kenlm/lm/Makefile
    src/kenlm/util/Makefile
    src/kenlm/search/Makefile
    src/include/Makefile
    src/lib/Makefile
    src/bin/Makefile
    src/test/Makefile
])

AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

AC_LANG_CPLUSPLUS
# LDFLAGS="$LDFLAGS -lpthread"
# CPPFLAGS="$CPPFLAGS -DKENLM_MAX_ORDER=6"

# Checks for libraries.
AC_SYS_LARGEFILE

# Checks for header files.
AC_CHECK_HEADERS([sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Check to make sure that we have unordered_map
AC_LANG([C++])
AC_CHECK_HEADERS([tr1/unordered_map])
AC_CHECK_HEADERS([ext/hash_map])

# Check if we have rt
AC_CHECK_LIB([rt], [timer_create], [AC_SUBST(LIBRT, "-lrt")], [])

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday pow sqrt])

AC_OUTPUT
