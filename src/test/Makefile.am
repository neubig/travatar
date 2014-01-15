include $(top_srcdir)/common.am
TRAVATARH = test-travatar.h test-base.h

AM_CXXFLAGS += -I$(srcdir)/../include -I$(srcdir)/../kenlm -I$(srcdir)/.. -DPKGDATADIR='"$(pkgdatadir)"'

noinst_PROGRAMS = test-travatar
TESTS = test-travatar

nobase_include_HEADERS = test-alignment.h test-base.h test-binarizer.h test-dict.h test-eval-measure.h test-graph-transformer.h test-hyper-graph.h test-io-util.h test-lookup-table.h test-rule-extractor.h test-travatar-runner.h test-tree-io.h test-trimmer.h test-tune.h test-weights.h test-hiero.h test-hiero-lookup-table.h

test_travatar_SOURCES = test-travatar.cc ${TRAVATARH}
test_travatar_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a ../tercpp/libter.a ../marisa/libmarisa.la ../liblbfgs/liblbfgs.la -lboost_thread -lboost_regex -lboost_iostreams -lz -lrt
