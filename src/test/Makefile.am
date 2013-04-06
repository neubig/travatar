TRABATARTH = test-travatar.h test-base.h

AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../kenlm -DPKGDATADIR='"$(pkgdatadir)"'

noinst_PROGRAMS = test-travatar

test_travatar_SOURCES = test-travatar.cc ${KYTH}
test_travatar_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a -lboost_thread -lboost_regex -lz -lmarisa -lrt
