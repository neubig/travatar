TRABATARTH = test-travatar.h test-base.h

AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../lib/kenlm -DPKGDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = test-travatar

test_travatar_SOURCES = test-travatar.cc ${KYTH}
test_travatar_LDADD = ../lib/libtravatar.la -lboost_regex -lz
