include $(top_srcdir)/common.am
TRAVATARH = test-travatar.h test-base.h

AM_CXXFLAGS += $(BOOST_CPPFLAGS) -I$(srcdir)/../include -I$(srcdir)/../kenlm -I$(srcdir)/.. -DPKGDATADIR='"$(pkgdatadir)"'

noinst_PROGRAMS = test-travatar
TESTS = test-travatar

nobase_include_HEADERS = \
	test-base.h

test_travatar_SOURCES = \
	test-travatar.cc \
	test-tokenizer.cc \
    test-alignment.cc \
    test-binarizer.cc \
    test-caser.cc \
    test-dict.cc \
    test-eval-measure.cc \
    test-graph-transformer.cc \
    test-hiero.cc \
    test-hyper-graph.cc \
    test-io-util.cc \
    test-lookup-table-fsm.cc \
	${TRAVATARH}
##    test-lookup-table.cc \
##    test-lm-composer.cc \
##    test-rule-extractor.cc \
##    test-tree-io.cc \
##    test-trimmer.cc \
##    test-tune.cc \
##    test-weights.cc \
##	test-math-query.cc

test_travatar_LDADD = \
	../lib/libtravatar.la \
	../kenlm/lm/libklm.la \
	../kenlm/util/libklm_util.la \
	../kenlm/search/libklm_search.la \
	../tercpp/libter.la \
	../marisa/libmarisa.la \
	../liblbfgs/liblbfgs.la \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_THREAD_LIBS) \
	$(BOOST_REGEX_LIBS) \
	$(BOOST_IOSTREAMS_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_LOCALE_LIBS) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIBS) \
	-lz
