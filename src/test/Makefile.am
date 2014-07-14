include $(top_srcdir)/common.am
TRAVATARH = test-travatar.h test-base.h

AM_CXXFLAGS += -I$(srcdir)/../include -I$(srcdir)/../kenlm -I$(srcdir)/.. -DPKGDATADIR='"$(pkgdatadir)"'

noinst_PROGRAMS = test-travatar
TESTS = test-travatar

nobase_include_HEADERS = \
	test-alignment.h \
	test-base.h \
	test-binarizer.h \
	test-dict.h \
	test-eval-measure.h \
	test-graph-transformer.h \
	test-hyper-graph.h \
	test-io-util.h \
	test-lookup-table.h \
	test-rule-extractor.h \
	test-travatar-runner.h \
	test-tree-io.h \
	test-trimmer.h \
	test-tune.h \
	test-weights.h \
	test-hiero.h \
	test-lookup-table-fsm.h

test_travatar_SOURCES = \
	test-travatar.cc \
	test-alignment.cc \
    test-binarizer.cc \
    test-dict.cc \
    test-eval-measure.cc \
    test-graph-transformer.cc \
    test-hiero.cc \
	${TRAVATARH}

test_travatar_LDADD = \
	../lib/libtravatar.la \
	../kenlm/lm/libklm.la \
	../kenlm/util/libklm_util.la \
	../kenlm/search/libklm_search.la \
	../tercpp/libter.la \
	../marisa/libmarisa.la \
	../liblbfgs/liblbfgs.la \
	-lboost_thread-mt \
	-lboost_regex-mt \
	-lboost_iostreams-mt \
	-lboost_system-mt \
	-lz ${LIBRT}
