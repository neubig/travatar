TRABATARTH = test-trabatar.h test-base.h

AM_CPPFLAGS = -I$(srcdir)/../include -DPKGDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = test-trabatar

test_trabatar_SOURCES = test-trabatar.cc ${KYTH}
test_trabatar_LDADD = ../lib/libtrabatar.la -lboost_regex
